/* Add AppImage magic bytes at offset 400 */
SECTIONS
{
  appimage.magic : ALIGN(0x400) {
    BYTE(0x41);
    BYTE(0x49);
    BYTE(0x03);
    BYTE(0x00);
    BYTE(0x41);
    BYTE(0x70);
    BYTE(0x70);
    BYTE(0x49);
    BYTE(0x6d);
    BYTE(0x61);
    BYTE(0x67);
    BYTE(0x65);
  }
}
INSERT AFTER .interp;
