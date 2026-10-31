/* ==========================================================================
   HỆ THỐNG QUẢN LÝ SIÊU THỊ MINI VÀ HÓA ĐƠN BÁN LẺ (POS)
   --------------------------------------------------------------------------
   Chương trình chạy trên cửa sổ dòng lệnh (console), viết bằng ngôn ngữ C.

   Chức năng:
     1. Bán hàng   : chọn món, tính tiền, in hóa đơn, tự trừ tồn kho
     2. Hàng hóa   : thêm / sửa / xóa / xem danh sách
     3. Hóa đơn    : xem lại danh sách và in lại từng hóa đơn
     4. Thống kê   : doanh thu hôm nay, tháng này, mặt hàng bán chạy

   Dữ liệu lưu trong 2 tệp văn bản: sanpham.txt và hoadon.txt
   ========================================================================== */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SANPHAM 200
#define MAX_HOADON 500
#define MAX_CHITIET 50
#define TEN_FILE_SP "sanpham.txt"
#define TEN_FILE_HD "hoadon.txt"

/* ------------------------------- cấu trúc dữ liệu ------------------------- */

typedef struct {
    char ma[10];
    char ten[60];
    char nhom[30];
    char donVi[12];
    long giaNhap;
    long giaBan;
    int ton;
} SanPham;

typedef struct {
    char ma[10];
    char ten[60];
    int soLuong;
    long donGia;
} ChiTiet;

typedef struct {
    char maHD[24];
    char thoiGian[20];     /* dạng dd/mm/yyyy hh:mm */
    char khach[50];
    char hinhThuc[20];
    ChiTiet chiTiet[MAX_CHITIET];
    int soMon;
    long tamTinh;
    long giamGia;
    long tong;
    long khachDua;
    long thoiLai;
} HoaDon;

/* ------------------------------- biến toàn cục ---------------------------- */

SanPham dsSanPham[MAX_SANPHAM];
int soSanPham = 0;

HoaDon dsHoaDon[MAX_HOADON];
int soHoaDon = 0;

/* ==========================================================================
   PHẦN 1. HÀM DÙNG CHUNG
   ========================================================================== */

/* Nhập một dòng chữ (bỏ ký tự xuống dòng ở cuối) */
void nhapChuoi(const char *nhac, char *ketQua, int kichThuoc)
{
    printf("%s", nhac);
    fflush(stdout);
    if (fgets(ketQua, kichThuoc, stdin) == NULL) {
        ketQua[0] = '\0';
        return;
    }
    ketQua[strcspn(ketQua, "\n")] = '\0';
}

/* Nhập một số nguyên. Trả về -1 nếu người dùng bấm Ctrl+D (hết dữ liệu) */
long nhapSo(const char *nhac)
{
    char dong[64];
    char *ketThuc;
    long giaTri;

    while (1) {
        printf("%s", nhac);
        fflush(stdout);
        if (fgets(dong, sizeof(dong), stdin) == NULL) {
            return -1;
        }
        ketThuc = NULL;
        giaTri = strtol(dong, &ketThuc, 10);
        if (ketThuc == dong) {
            printf("  -> Vui long nhap so.\n");
            continue;
        }
        return giaTri;
    }
}

/* Như nhapSo nhưng cho phép bỏ trống: trả về giaTriMacDinh */
long nhapSoTuyChon(const char *nhac, long giaTriMacDinh)
{
    char dong[64];
    char *ketThuc;
    long giaTri;

    printf("%s", nhac);
    fflush(stdout);
    if (fgets(dong, sizeof(dong), stdin) == NULL) return giaTriMacDinh;

    ketThuc = NULL;
    giaTri = strtol(dong, &ketThuc, 10);
    if (ketThuc == dong) return giaTriMacDinh;   /* bỏ trống -> giữ nguyên */
    return giaTri;
}

/* Đổi số tiền thành chuỗi có dấu chấm ngăn cách: 125000 -> "125.000" */
void dinhDangTien(long soTien, char *ketQua)
{
    char tam[32];
    int doDai, i, j = 0;

    sprintf(tam, "%ld", soTien);
    doDai = (int)strlen(tam);

    for (i = 0; i < doDai; i++) {
        ketQua[j++] = tam[i];
        if ((doDai - 1 - i) % 3 == 0 && i != doDai - 1) {
            ketQua[j++] = '.';
        }
    }
    ketQua[j] = '\0';
}

/* In một dòng kẻ */
void inDongKe(char kyTu, int soLuong)
{
    int i;
    for (i = 0; i < soLuong; i++) putchar(kyTu);
    putchar('\n');
}

/* Lấy thời gian hiện tại dạng dd/mm/yyyy hh:mm */
void layThoiGian(char *ketQua, int kichThuoc)
{
    time_t bayGio = time(NULL);
    struct tm *moc = localtime(&bayGio);
    strftime(ketQua, kichThuoc, "%d/%m/%Y %H:%M", moc);
}

/* Lấy ngày hôm nay dạng yyyymmdd (dùng để đặt mã hóa đơn) */
void layNgayMa(char *ketQua, int kichThuoc)
{
    time_t bayGio = time(NULL);
    struct tm *moc = localtime(&bayGio);
    strftime(ketQua, kichThuoc, "%Y%m%d", moc);
}

/* Tìm sản phẩm theo mã, trả về chỉ số trong mảng, -1 nếu không thấy */
int timSanPham(const char *ma)
{
    int i;
    for (i = 0; i < soSanPham; i++) {
        if (strcmp(dsSanPham[i].ma, ma) == 0) return i;
    }
    return -1;
}

/* Tạo mã sản phẩm kế tiếp: SP001, SP002, ... */
void taoMaSanPham(char *ketQua)
{
    int i, soLonNhat = 0, so;
    for (i = 0; i < soSanPham; i++) {
        so = atoi(dsSanPham[i].ma + 2);   /* bỏ 2 ký tự "SP" */
        if (so > soLonNhat) soLonNhat = so;
    }
    sprintf(ketQua, "SP%03d", soLonNhat + 1);
}

/* Tạo mã hóa đơn: HD + ngày + số thứ tự trong ngày */
void taoMaHoaDon(char *ketQua)
{
    char ngay[12], tienTo[24];
    int i, dem = 0;

    layNgayMa(ngay, sizeof(ngay));
    sprintf(tienTo, "HD%s", ngay);

    for (i = 0; i < soHoaDon; i++) {
        if (strncmp(dsHoaDon[i].maHD, tienTo, strlen(tienTo)) == 0) dem++;
    }
    sprintf(ketQua, "%s-%03d", tienTo, dem + 1);
}

/* ==========================================================================
   PHẦN 2. ĐỌC / GHI TỆP
   ========================================================================== */

/* Đọc danh sách sản phẩm từ tệp. Trả về số dòng đọc được. */
int docSanPham(void)
{
    FILE *f = fopen(TEN_FILE_SP, "r");
    char dong[256];

    soSanPham = 0;
    if (f == NULL) return 0;

    while (fgets(dong, sizeof(dong), f) != NULL && soSanPham < MAX_SANPHAM) {
        SanPham *sp = &dsSanPham[soSanPham];
        dong[strcspn(dong, "\r\n")] = '\0';
        if (strlen(dong) == 0) continue;

        /* mỗi dòng có dạng: ma|ten|nhom|donVi|giaNhap|giaBan|ton */
        if (sscanf(dong, "%9[^|]|%59[^|]|%29[^|]|%11[^|]|%ld|%ld|%d",
                   sp->ma, sp->ten, sp->nhom, sp->donVi,
                   &sp->giaNhap, &sp->giaBan, &sp->ton) == 7) {
            soSanPham++;
        }
    }
    fclose(f);
    return soSanPham;
}

/* Ghi toàn bộ danh sách sản phẩm ra tệp */
void ghiSanPham(void)
{
    FILE *f = fopen(TEN_FILE_SP, "w");
    int i;

    if (f == NULL) {
        printf("Loi: khong ghi duoc tep %s\n", TEN_FILE_SP);
        return;
    }
    for (i = 0; i < soSanPham; i++) {
        fprintf(f, "%s|%s|%s|%s|%ld|%ld|%d\n",
                dsSanPham[i].ma, dsSanPham[i].ten, dsSanPham[i].nhom,
                dsSanPham[i].donVi, dsSanPham[i].giaNhap,
                dsSanPham[i].giaBan, dsSanPham[i].ton);
    }
    fclose(f);
}

/* Đọc hóa đơn đã bán từ tệp */
int docHoaDon(void)
{
    FILE *f = fopen(TEN_FILE_HD, "r");
    char dong[300];

    soHoaDon = 0;
    if (f == NULL) return 0;

    while (fgets(dong, sizeof(dong), f) != NULL && soHoaDon < MAX_HOADON) {
        HoaDon *hd = &dsHoaDon[soHoaDon];
        dong[strcspn(dong, "\r\n")] = '\0';
        if (strlen(dong) == 0) continue;

        /* dòng đầu của hóa đơn:
           maHD|thoiGian|khach|hinhThuc|tamTinh|giamGia|tong|khachDua|thoiLai */
        if (sscanf(dong, "%23[^|]|%19[^|]|%49[^|]|%19[^|]|%ld|%ld|%ld|%ld|%ld",
                   hd->maHD, hd->thoiGian, hd->khach, hd->hinhThuc,
                   &hd->tamTinh, &hd->giamGia, &hd->tong,
                   &hd->khachDua, &hd->thoiLai) != 9) {
            continue;
        }

        /* các dòng tiếp theo là mặt hàng, đến khi gặp dòng trống */
        hd->soMon = 0;
        while (fgets(dong, sizeof(dong), f) != NULL) {
            dong[strcspn(dong, "\r\n")] = '\0';
            if (strlen(dong) == 0) break;
            if (hd->soMon >= MAX_CHITIET) continue;

            if (sscanf(dong, "%9[^|]|%59[^|]|%d|%ld",
                       hd->chiTiet[hd->soMon].ma, hd->chiTiet[hd->soMon].ten,
                       &hd->chiTiet[hd->soMon].soLuong,
                       &hd->chiTiet[hd->soMon].donGia) == 4) {
                hd->soMon++;
            }
        }
        soHoaDon++;
    }
    fclose(f);
    return soHoaDon;
}

/* Ghi thêm một hóa đơn vào cuối tệp */
void ghiThemHoaDon(const HoaDon *hd)
{
    FILE *f = fopen(TEN_FILE_HD, "a");
    int i;

    if (f == NULL) {
        printf("Loi: khong ghi duoc tep %s\n", TEN_FILE_HD);
        return;
    }

    fprintf(f, "%s|%s|%s|%s|%ld|%ld|%ld|%ld|%ld\n",
            hd->maHD, hd->thoiGian, hd->khach, hd->hinhThuc,
            hd->tamTinh, hd->giamGia, hd->tong, hd->khachDua, hd->thoiLai);

    for (i = 0; i < hd->soMon; i++) {
        fprintf(f, "%s|%s|%d|%ld\n",
                hd->chiTiet[i].ma, hd->chiTiet[i].ten,
                hd->chiTiet[i].soLuong, hd->chiTiet[i].donGia);
    }
    fprintf(f, "\n");
    fclose(f);
}

/* Nếu chưa có dữ liệu thì nạp sẵn vài mặt hàng mẫu */
void napDuLieuMau(void)
{
    struct { const char *ma, *ten, *nhom, *dvt; long giaNhap, giaBan; int ton; } mau[] = {
        {"SP001", "Sua tuoi Vinamilk 1L",      "Sua",            "Hop", 24000, 32000, 40},
        {"SP002", "Mi Hao Hao tom chua cay",   "Mi goi",         "Goi",  3500,  5000, 120},
        {"SP003", "Nuoc suoi Aquafina 500ml",  "Nuoc uong",      "Chai", 4000,  7000, 80},
        {"SP004", "Coca-Cola lon 330ml",       "Nuoc uong",      "Lon",  8000, 12000, 60},
        {"SP005", "Banh mi sandwich",          "Banh",           "Goi", 12000, 18000, 25},
        {"SP006", "Trung ga ta (10 qua)",      "Thuc pham",      "Vi",  28000, 38000, 15},
        {"SP007", "Gao ST25 tui 2kg",          "Thuc pham",      "Tui", 52000, 68000, 18},
        {"SP008", "Dau an Simply 1L",          "Gia vi",         "Chai",42000, 55000, 12},
        {"SP009", "Nuoc mam Nam Ngu 500ml",    "Gia vi",         "Chai",26000, 35000, 20},
        {"SP010", "Kem danh rang PS 180g",     "Hoa my pham",    "Tuyp",22000, 31000, 16},
        {"SP011", "Giay an Bless You",         "Hoa my pham",    "Goi",  9000, 14000, 35},
        {"SP012", "Banh Oreo goi 133g",        "Banh",           "Goi", 16000, 23000,  4},
        {"SP013", "Ca phe G7 hop 20 goi",      "Do uong",        "Hop", 48000, 65000,  9},
        {"SP014", "Sua chua Vinamilk (loc 4)", "Sua",            "Loc", 20000, 28000, 22}
    };
    int n = (int)(sizeof(mau) / sizeof(mau[0]));
    int i;

    for (i = 0; i < n && soSanPham < MAX_SANPHAM; i++) {
        SanPham *sp = &dsSanPham[soSanPham++];
        strcpy(sp->ma, mau[i].ma);
        strcpy(sp->ten, mau[i].ten);
        strcpy(sp->nhom, mau[i].nhom);
        strcpy(sp->donVi, mau[i].dvt);
        sp->giaNhap = mau[i].giaNhap;
        sp->giaBan = mau[i].giaBan;
        sp->ton = mau[i].ton;
    }
    ghiSanPham();
}

/* ==========================================================================
   PHẦN 3. HIỂN THỊ
   ========================================================================== */

void inTieuDe(const char *tieuDe)
{
    printf("\n");
    inDongKe('=', 78);
    printf("  %s\n", tieuDe);
    inDongKe('=', 78);
}

/* In bảng danh sách sản phẩm */
void inDanhSachSanPham(void)
{
    int i;
    char giaNhap[32], giaBan[32];

    if (soSanPham == 0) {
        printf("  (Chua co san pham nao)\n");
        return;
    }

    printf("  %-3s %-7s %-30s %-12s %12s %12s %6s\n",
           "STT", "Ma", "Ten hang", "Nhom", "Gia ban", "Gia nhap", "Ton");
    inDongKe('-', 96);

    for (i = 0; i < soSanPham; i++) {
        dinhDangTien(dsSanPham[i].giaBan, giaBan);
        dinhDangTien(dsSanPham[i].giaNhap, giaNhap);
        printf("  %-3d %-7s %-30s %-12s %10s d %12s d %6d\n",
               i + 1, dsSanPham[i].ma, dsSanPham[i].ten, dsSanPham[i].nhom,
               giaBan, giaNhap, dsSanPham[i].ton);
    }
}

/* In một hóa đơn ra màn hình */
void inHoaDon(const HoaDon *hd)
{
    char tamTinh[32], giamGia[32], tong[32], khachDua[32], thoiLai[32];
    int i;

    dinhDangTien(hd->tamTinh, tamTinh);
    dinhDangTien(hd->giamGia, giamGia);
    dinhDangTien(hd->tong, tong);
    dinhDangTien(hd->khachDua, khachDua);
    dinhDangTien(hd->thoiLai, thoiLai);

    printf("\n");
    inDongKe('-', 52);
    printf("        SIEU THI MINI AN BINH\n");
    printf("   123 Nguyen Van Cu, Quan 5, TP.HCM\n");
    printf("          DT: 0909 123 456\n");
    inDongKe('-', 52);
    printf("           HOA DON BAN LE\n");
    inDongKe('-', 52);
    printf("  So HD : %s\n", hd->maHD);
    printf("  Ngay  : %s\n", hd->thoiGian);
    printf("  Khach : %s\n", hd->khach);
    inDongKe('-', 52);
    printf("  %-24s %5s %12s\n", "Mat hang", "SL", "Thanh tien");

    for (i = 0; i < hd->soMon; i++) {
        char thanhTien[32], donGia[32];
        dinhDangTien((long)hd->chiTiet[i].soLuong * hd->chiTiet[i].donGia, thanhTien);
        dinhDangTien(hd->chiTiet[i].donGia, donGia);
        printf("  %-24s %5d %12s d\n", hd->chiTiet[i].ten,
               hd->chiTiet[i].soLuong, thanhTien);
        printf("  %-24s      %10s d/1\n", "", donGia);
    }

    inDongKe('-', 52);
    printf("  %-30s %16s d\n", "Tam tinh:", tamTinh);
    if (hd->giamGia > 0) {
        printf("  %-30s %16s d\n", "Giam gia:", giamGia);
    }
    printf("  %-30s %16s d\n", "TONG CONG:", tong);
    printf("  %-30s %16s d\n", hd->hinhThuc, khachDua);
    printf("  %-30s %16s d\n", "Tien thoi lai:", thoiLai);
    inDongKe('-', 52);
    printf("     Cam on quy khach, hen gap lai!\n");
    inDongKe('-', 52);
}

/* ==========================================================================
   PHẦN 4. QUẢN LÝ HÀNG HÓA
   ========================================================================== */

void themSanPham(void)
{
    SanPham sp;

    if (soSanPham >= MAX_SANPHAM) {
        printf("  Danh sach da day, khong them duoc.\n");
        return;
    }

    inTieuDe("THEM SAN PHAM MOI");
    taoMaSanPham(sp.ma);
    printf("  Ma san pham (tu dong): %s\n", sp.ma);

    nhapChuoi("  Ten hang    : ", sp.ten, sizeof(sp.ten));
    if (strlen(sp.ten) == 0) {
        printf("  -> Chua nhap ten, da huy.\n");
        return;
    }
    nhapChuoi("  Nhom hang   : ", sp.nhom, sizeof(sp.nhom));
    nhapChuoi("  Don vi tinh : ", sp.donVi, sizeof(sp.donVi));

    sp.giaNhap = nhapSo("  Gia nhap    : ");
    sp.giaBan  = nhapSo("  Gia ban     : ");
    sp.ton     = (int)nhapSo("  So luong ton: ");

    if (sp.giaBan <= 0 || sp.ton < 0) {
        printf("  -> Gia ban phai lon hon 0, ton khong duoc am. Da huy.\n");
        return;
    }

    dsSanPham[soSanPham++] = sp;
    ghiSanPham();
    printf("  Da them: %s - %s\n", sp.ma, sp.ten);
}

void suaSanPham(void)
{
    char ma[10];
    int viTri;
    SanPham *sp;

    inTieuDe("SUA THONG TIN SAN PHAM");
    inDanhSachSanPham();

    nhapChuoi("\n  Nhap ma san pham can sua: ", ma, sizeof(ma));
    viTri = timSanPham(ma);
    if (viTri < 0) {
        printf("  -> Khong tim thay ma %s\n", ma);
        return;
    }

    sp = &dsSanPham[viTri];
    printf("  Dang sua: %s - %s (bo trong de giu nguyen)\n", sp->ma, sp->ten);

    {
        char tam[60];
        nhapChuoi("  Ten hang moi : ", tam, sizeof(tam));
        if (strlen(tam) > 0) strcpy(sp->ten, tam);

        nhapChuoi("  Nhom hang moi: ", tam, sizeof(tam));
        if (strlen(tam) > 0) strcpy(sp->nhom, tam);

        nhapChuoi("  Don vi moi   : ", tam, sizeof(tam));
        if (strlen(tam) > 0) strcpy(sp->donVi, tam);
    }

    {
        long so = nhapSoTuyChon("  Gia nhap moi (bo trong = giu nguyen): ", sp->giaNhap);
        if (so >= 0) sp->giaNhap = so;

        so = nhapSoTuyChon("  Gia ban moi (bo trong = giu nguyen) : ", sp->giaBan);
        if (so > 0) sp->giaBan = so;

        so = nhapSoTuyChon("  Ton kho moi (bo trong = giu nguyen) : ", sp->ton);
        if (so >= 0) sp->ton = (int)so;
    }

    ghiSanPham();
    printf("  Da cap nhat san pham %s\n", sp->ma);
}

void xoaSanPham(void)
{
    char ma[10], traLoi[8];
    int viTri, i;

    inTieuDe("XOA SAN PHAM");
    inDanhSachSanPham();

    nhapChuoi("\n  Nhap ma san pham can xoa: ", ma, sizeof(ma));
    viTri = timSanPham(ma);
    if (viTri < 0) {
        printf("  -> Khong tim thay ma %s\n", ma);
        return;
    }

    printf("  Sap xoa: %s - %s\n", dsSanPham[viTri].ma, dsSanPham[viTri].ten);
    nhapChuoi("  Ban chac chan? (c/k): ", traLoi, sizeof(traLoi));
    if (traLoi[0] != 'c' && traLoi[0] != 'C') {
        printf("  Da huy.\n");
        return;
    }

    for (i = viTri; i < soSanPham - 1; i++) {
        dsSanPham[i] = dsSanPham[i + 1];
    }
    soSanPham--;
    ghiSanPham();
    printf("  Da xoa san pham %s\n", ma);
}

void menuHangHoa(void)
{
    int chon;

    do {
        inTieuDe("QUAN LY HANG HOA");
        printf("  1. Xem danh sach\n");
        printf("  2. Them san pham\n");
        printf("  3. Sua san pham\n");
        printf("  4. Xoa san pham\n");
        printf("  0. Quay lai\n");
        inDongKe('-', 78);

        chon = (int)nhapSo("  Chon: ");
        if (chon == -1) return;

        switch (chon) {
        case 1: inTieuDe("DANH SACH SAN PHAM"); inDanhSachSanPham(); break;
        case 2: themSanPham(); break;
        case 3: suaSanPham(); break;
        case 4: xoaSanPham(); break;
        case 0: break;
        default: printf("  -> Lua chon khong hop le.\n");
        }
    } while (chon != 0);
}

/* ==========================================================================
   PHẦN 5. BÁN HÀNG
   ========================================================================== */

void banHang(void)
{
    ChiTiet gioHang[MAX_CHITIET];
    int soMon = 0;
    int chon, viTri, i;
    long tamTinh = 0, giamGia = 0, tong, khachDua, thoiLai = 0;
    int hinhThuc;
    HoaDon hd;

    if (soSanPham == 0) {
        printf("  Chua co san pham nao. Hay them hang truoc.\n");
        return;
    }

    inTieuDe("BAN HANG");
    inDanhSachSanPham();

    /* --- chọn hàng vào giỏ --- */
    while (1) {
        printf("\n");
        chon = (int)nhapSo("  Chon so thu tu san pham (0 = xong): ");
        if (chon == 0 || chon == -1) break;
        if (chon < 1 || chon > soSanPham) {
            printf("  -> So thu tu khong hop le.\n");
            continue;
        }

        viTri = chon - 1;
        {
            int soLuong = (int)nhapSo("  So luong: ");
            if (soLuong <= 0) {
                printf("  -> So luong phai lon hon 0.\n");
                continue;
            }
            if (soLuong > dsSanPham[viTri].ton) {
                printf("  -> Khong du hang: %s chi con %d %s\n",
                       dsSanPham[viTri].ten, dsSanPham[viTri].ton,
                       dsSanPham[viTri].donVi);
                continue;
            }

            /* nếu đã có trong giỏ thì cộng dồn */
            {
                int daCo = -1, j;
                for (j = 0; j < soMon; j++) {
                    if (strcmp(gioHang[j].ma, dsSanPham[viTri].ma) == 0) { daCo = j; break; }
                }
                if (daCo >= 0) {
                    if (gioHang[daCo].soLuong + soLuong > dsSanPham[viTri].ton) {
                        printf("  -> Vuot qua so luong con lai trong kho.\n");
                        continue;
                    }
                    gioHang[daCo].soLuong += soLuong;
                } else {
                    if (soMon >= MAX_CHITIET) {
                        printf("  -> Gio hang da day.\n");
                        break;
                    }
                    strcpy(gioHang[soMon].ma, dsSanPham[viTri].ma);
                    strcpy(gioHang[soMon].ten, dsSanPham[viTri].ten);
                    gioHang[soMon].soLuong = soLuong;
                    gioHang[soMon].donGia = dsSanPham[viTri].giaBan;
                    soMon++;
                }
            }
        }
        printf("  Da them vao gio.\n");
    }

    if (soMon == 0) {
        printf("  Gio hang trong, khong thanh toan.\n");
        return;
    }

    /* --- tính tiền --- */
    for (i = 0; i < soMon; i++) {
        tamTinh += (long)gioHang[i].soLuong * gioHang[i].donGia;
    }

    printf("\n");
    inDongKe('-', 52);
    {
        char tien[32];
        dinhDangTien(tamTinh, tien);
        printf("  Tam tinh: %s d\n", tien);
    }

    giamGia = nhapSoTuyChon("  Giam gia (bo trong = 0): ", 0);
    if (giamGia < 0) giamGia = 0;
    if (giamGia > tamTinh) giamGia = tamTinh;

    tong = tamTinh - giamGia;
    {
        char tien[32];
        dinhDangTien(tong, tien);
        printf("  Khach phai tra: %s d\n", tien);
    }

    /* --- thanh toán --- */
    printf("  Hinh thuc: 1. Tien mat  2. Chuyen khoan  3. The\n");
    hinhThuc = (int)nhapSo("  Chon: ");

    if (hinhThuc == 1) {
        do {
            khachDua = nhapSo("  Tien khach dua: ");
            if (khachDua < tong) printf("  -> Chua du tien, nhap lai.\n");
        } while (khachDua < tong);
        thoiLai = khachDua - tong;
    } else {
        khachDua = tong;
        thoiLai = 0;
    }

    /* --- tạo hóa đơn --- */
    memset(&hd, 0, sizeof(hd));
    taoMaHoaDon(hd.maHD);
    layThoiGian(hd.thoiGian, sizeof(hd.thoiGian));
    nhapChuoi("  Ten khach (bo trong = Khach le): ", hd.khach, sizeof(hd.khach));
    if (strlen(hd.khach) == 0) strcpy(hd.khach, "Khach le");

    if (hinhThuc == 1) strcpy(hd.hinhThuc, "Tien mat");
    else if (hinhThuc == 2) strcpy(hd.hinhThuc, "Chuyen khoan");
    else strcpy(hd.hinhThuc, "The");

    for (i = 0; i < soMon; i++) hd.chiTiet[i] = gioHang[i];
    hd.soMon = soMon;
    hd.tamTinh = tamTinh;
    hd.giamGia = giamGia;
    hd.tong = tong;
    hd.khachDua = khachDua;
    hd.thoiLai = thoiLai;

    /* --- trừ tồn kho --- */
    for (i = 0; i < soMon; i++) {
        viTri = timSanPham(gioHang[i].ma);
        if (viTri >= 0) dsSanPham[viTri].ton -= gioHang[i].soLuong;
    }
    ghiSanPham();

    /* --- lưu và in hóa đơn --- */
    if (soHoaDon < MAX_HOADON) dsHoaDon[soHoaDon++] = hd;
    ghiThemHoaDon(&hd);
    inHoaDon(&hd);

    printf("\n  Da luu hoa don %s\n", hd.maHD);
}

/* ==========================================================================
   PHẦN 6. XEM LẠI HÓA ĐƠN
   ========================================================================== */

void inDanhSachHoaDon(void)
{
    int i;
    char tien[32];

    if (soHoaDon == 0) {
        printf("  (Chua co hoa don nao)\n");
        return;
    }

    printf("  %-3s %-18s %-18s %-12s %12s\n",
           "STT", "Ma hoa don", "Thoi gian", "Thanh toan", "Tong tien");
    inDongKe('-', 74);

    for (i = 0; i < soHoaDon; i++) {
        dinhDangTien(dsHoaDon[i].tong, tien);
        printf("  %-3d %-18s %-18s %-12s %10s d\n",
               i + 1, dsHoaDon[i].maHD, dsHoaDon[i].thoiGian,
               dsHoaDon[i].hinhThuc, tien);
    }
}

void menuHoaDon(void)
{
    int chon, viTri;

    do {
        inTieuDe("DANH SACH HOA DON");
        inDanhSachHoaDon();
        printf("\n  1. Xem chi tiet mot hoa don\n");
        printf("  0. Quay lai\n");
        inDongKe('-', 78);

        chon = (int)nhapSo("  Chon: ");
        if (chon == -1) return;
        if (chon == 1) {
            viTri = (int)nhapSo("  Nhap so thu tu hoa don: ");
            if (viTri >= 1 && viTri <= soHoaDon) {
                inHoaDon(&dsHoaDon[viTri - 1]);
            } else {
                printf("  -> Khong hop le.\n");
            }
        }
    } while (chon != 0);
}

/* ==========================================================================
   PHẦN 7. THỐNG KÊ
   ========================================================================== */

/* Đếm số hóa đơn và doanh thu theo tiền tố ngày (dd/mm/yyyy) */
long doanhThuTheoNgay(const char *tienTo, int *soLuong)
{
    int i;
    long tong = 0;

    *soLuong = 0;
    for (i = 0; i < soHoaDon; i++) {
        if (strncmp(dsHoaDon[i].thoiGian, tienTo, strlen(tienTo)) == 0) {
            tong += dsHoaDon[i].tong;
            (*soLuong)++;
        }
    }
    return tong;
}

void thongKe(void)
{
    char tam[32];
    int soLuong, i, j;
    long doanhThu;

    /* cấu trúc phụ để gom nhóm mặt hàng bán chạy */
    struct { char ma[10]; char ten[60]; int soLuong; long doanhThu; } banChay[MAX_SANPHAM];
    int soBanChay = 0;

    inTieuDe("THONG KE BAN HANG");

    /* --- doanh thu hôm nay --- */
    {
        char ngayHienThi[12];
        time_t bayGio = time(NULL);
        struct tm *moc = localtime(&bayGio);
        strftime(ngayHienThi, sizeof(ngayHienThi), "%d/%m/%Y", moc);

        doanhThu = doanhThuTheoNgay(ngayHienThi, &soLuong);
    }
    dinhDangTien(doanhThu, tam);
    printf("  Doanh thu hom nay      : %s d  (%d hoa don)\n", tam, soLuong);

    /* --- doanh thu tháng này: so khớp phần "/mm/yyyy" --- */
    {
        time_t bayGio = time(NULL);
        struct tm *moc = localtime(&bayGio);
        {
            char duoiThang[12];
            int dem = 0;
            long tong = 0;
            sprintf(duoiThang, "/%02d/%04d", moc->tm_mon + 1, moc->tm_year + 1900);
            for (i = 0; i < soHoaDon; i++) {
                if (strstr(dsHoaDon[i].thoiGian, duoiThang) != NULL) {
                    tong += dsHoaDon[i].tong;
                    dem++;
                }
            }
            dinhDangTien(tong, tam);
            printf("  Doanh thu thang nay    : %s d  (%d hoa don)\n", tam, dem);
        }
    }

    /* --- tồn kho --- */
    {
        long giaTriTon = 0;
        int sapHet = 0;
        for (i = 0; i < soSanPham; i++) {
            giaTriTon += (long)dsSanPham[i].giaNhap * dsSanPham[i].ton;
            if (dsSanPham[i].ton <= 5) sapHet++;
        }
        dinhDangTien(giaTriTon, tam);
        printf("  Gia tri ton kho        : %s d\n", tam);
        printf("  Mat hang sap het (<=5) : %d\n", sapHet);
    }

    /* --- gom nhóm mặt hàng bán chạy --- */
    for (i = 0; i < soHoaDon; i++) {
        for (j = 0; j < dsHoaDon[i].soMon; j++) {
            int k, timThay = -1;
            for (k = 0; k < soBanChay; k++) {
                if (strcmp(banChay[k].ma, dsHoaDon[i].chiTiet[j].ma) == 0) { timThay = k; break; }
            }
            if (timThay < 0 && soBanChay < MAX_SANPHAM) {
                strcpy(banChay[soBanChay].ma, dsHoaDon[i].chiTiet[j].ma);
                strcpy(banChay[soBanChay].ten, dsHoaDon[i].chiTiet[j].ten);
                banChay[soBanChay].soLuong = 0;
                banChay[soBanChay].doanhThu = 0;
                timThay = soBanChay++;
            }
            if (timThay >= 0) {
                banChay[timThay].soLuong += dsHoaDon[i].chiTiet[j].soLuong;
                banChay[timThay].doanhThu +=
                    (long)dsHoaDon[i].chiTiet[j].soLuong * dsHoaDon[i].chiTiet[j].donGia;
            }
        }
    }

    /* sắp xếp giảm dần bằng sắp xếp chọn */
    for (i = 0; i < soBanChay - 1; i++) {
        int lonNhat = i, k;
        for (k = i + 1; k < soBanChay; k++) {
            if (banChay[k].soLuong > banChay[lonNhat].soLuong) lonNhat = k;
        }
        if (lonNhat != i) {
            struct { char ma[10]; char ten[60]; int soLuong; long doanhThu; } tamDoi;
            memcpy(&tamDoi, &banChay[i], sizeof(tamDoi));
            memcpy(&banChay[i], &banChay[lonNhat], sizeof(tamDoi));
            memcpy(&banChay[lonNhat], &tamDoi, sizeof(tamDoi));
        }
    }

    printf("\n  BAN CHAY (toi da 5 mat hang):\n");
    if (soBanChay == 0) {
        printf("    (Chua ban duoc mat hang nao)\n");
    } else {
        printf("  %-3s %-30s %8s %14s\n", "STT", "Ten hang", "Da ban", "Doanh thu");
        inDongKe('-', 62);
        for (i = 0; i < soBanChay && i < 5; i++) {
            dinhDangTien(banChay[i].doanhThu, tam);
            printf("  %-3d %-30s %8d %12s d\n",
                   i + 1, banChay[i].ten, banChay[i].soLuong, tam);
        }
    }

    /* --- mặt hàng sắp hết --- */
    printf("\n  MAT HANG SAP HET:\n");
    {
        int co = 0;
        for (i = 0; i < soSanPham; i++) {
            if (dsSanPham[i].ton <= 5) {
                printf("    - %-30s con %d %s\n", dsSanPham[i].ten,
                       dsSanPham[i].ton, dsSanPham[i].donVi);
                co = 1;
            }
        }
        if (!co) printf("    (Khong co mat hang nao)\n");
    }
}

/* ==========================================================================
   PHẦN 8. CHƯƠNG TRÌNH CHÍNH
   ========================================================================== */

int main(void)
{
    int chon;

    docSanPham();
    docHoaDon();
    if (soSanPham == 0) {
        printf("Lan dau chay: dang nap 14 mat hang mau...\n");
        napDuLieuMau();
    }

    do {
        printf("\n");
        inDongKe('=', 78);
        printf("      HE THONG QUAN LY SIEU THI MINI - HOA DON BAN LE\n");
        inDongKe('=', 78);
        printf("  1. Ban hang\n");
        printf("  2. Quan ly hang hoa\n");
        printf("  3. Xem hoa don\n");
        printf("  4. Thong ke\n");
        printf("  0. Thoat\n");
        inDongKe('-', 78);
        printf("  Dang co %d mat hang, %d hoa don.\n", soSanPham, soHoaDon);

        chon = (int)nhapSo("  Chon: ");
        if (chon == -1) break;

        switch (chon) {
        case 1: banHang(); break;
        case 2: menuHangHoa(); break;
        case 3: menuHoaDon(); break;
        case 4: thongKe(); break;
        case 0:
            printf("\n  Da luu du lieu. Tam biet!\n");
            break;
        default:
            printf("  -> Lua chon khong hop le.\n");
        }
    } while (chon != 0);

    ghiSanPham();
    return 0;
}
