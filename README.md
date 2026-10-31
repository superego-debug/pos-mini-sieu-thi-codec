# Hệ thống quản lý siêu thị mini và hóa đơn bán lẻ (POS)

Chương trình quản lý bán hàng viết bằng **ngôn ngữ C**, chạy trên cửa sổ dòng lệnh.
Dùng cho đồ án môn học C / Kỹ thuật lập trình.

## Chức năng

**1. Bán hàng**
- Xem danh sách hàng, chọn món theo số thứ tự và nhập số lượng
- Chặn không cho bán vượt số lượng còn trong kho
- Tính tạm tính, giảm giá, tổng tiền
- Thanh toán tiền mặt / chuyển khoản / thẻ; nhập tiền khách đưa, tự tính tiền thối
- In hóa đơn ra màn hình, tự trừ tồn kho và lưu lại

**2. Quản lý hàng hóa**
- Xem danh sách, thêm, sửa, xóa mặt hàng
- Mã hàng tự sinh: SP001, SP002…
- Sửa thì bỏ trống ô nào là giữ nguyên ô đó

**3. Hóa đơn**
- Xem danh sách hóa đơn đã bán
- Chọn một hóa đơn để xem lại và in lại

**4. Thống kê**
- Doanh thu hôm nay, doanh thu tháng này (kèm số hóa đơn)
- Giá trị hàng đang tồn kho (theo giá nhập)
- Top 5 mặt hàng bán chạy
- Danh sách mặt hàng sắp hết (tồn ≤ 5)

## Cách biên dịch và chạy

Cần có trình biên dịch C (gcc, clang…).

**Cách 1 — dùng make:**

```bash
make
make run
```

**Cách 2 — biên dịch trực tiếp:**

```bash
gcc -std=c99 -Wall -Wextra -O2 -o pos main.c
./pos
```

Trên Windows (dùng MinGW hoặc Dev-C++) chỉ cần mở `main.c` rồi bấm Build & Run,
hoặc gõ `gcc -o pos.exe main.c`.

## Tệp dữ liệu

Chương trình tạo và đọc 2 tệp văn bản, nằm cùng thư mục chạy:

**`sanpham.txt`** — mỗi dòng một mặt hàng, các trường ngăn bằng dấu `|`:

```
ma|ten|nhom|donVi|giaNhap|giaBan|ton
SP001|Sua tuoi Vinamilk 1L|Sua|Hop|24000|32000|40
```

**`hoadon.txt`** — mỗi hóa đơn gồm một dòng đầu và các dòng mặt hàng, kết thúc bằng dòng trống:

```
maHD|thoiGian|khach|hinhThuc|tamTinh|giamGia|tong|khachDua|thoiLai
ma|ten|soLuong|donGia
```

Lần đầu chạy (chưa có `sanpham.txt`), chương trình tự nạp 14 mặt hàng mẫu.

## Cấu trúc chương trình

| Phần | Nội dung |
|---|---|
| 1 | Hàm dùng chung: nhập dữ liệu, định dạng tiền, thời gian, tìm hàng |
| 2 | Đọc / ghi tệp |
| 3 | Hiển thị: bảng hàng hóa, hóa đơn |
| 4 | Quản lý hàng hóa (thêm, sửa, xóa) |
| 5 | Bán hàng |
| 6 | Xem lại hóa đơn |
| 7 | Thống kê |
| 8 | Chương trình chính (`main`) |

Ba kiểu dữ liệu chính:

```c
typedef struct { char ma[10], ten[60], nhom[30], donVi[12];
                 long giaNhap, giaBan; int ton; } SanPham;

typedef struct { char ma[10], ten[60]; int soLuong; long donGia; } ChiTiet;

typedef struct { char maHD[24], thoiGian[20], khach[50], hinhThuc[20];
                 ChiTiet chiTiet[50]; int soMon;
                 long tamTinh, giamGia, tong, khachDua, thoiLai; } HoaDon;
```

## Ghi chú

- Chương trình in chữ **không dấu** để hiển thị đúng trên mọi máy, kể cả Command Prompt
  của Windows (nếu in tiếng Việt có dấu, cmd thường bị lỗi font).
- Dữ liệu lưu bằng tệp văn bản nên mở bằng Notepad vẫn đọc được, tiện kiểm tra khi bảo vệ.
- Muốn xóa dữ liệu chạy lại từ đầu: xóa `sanpham.txt` và `hoadon.txt`.

## Hướng phát triển

- Thêm phiếu nhập hàng và quản lý nhà cung cấp
- Đăng nhập cho thu ngân và quản lý
- Xuất báo cáo doanh thu ra tệp CSV để mở bằng Excel
- In hóa đơn ra tệp thay vì chỉ in ra màn hình
