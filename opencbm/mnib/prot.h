void shift_buffer_left(BYTE *buffer, int length, int n);
void shift_buffer_right(BYTE *buffer, int length, int n);
BYTE* align_vmax(BYTE *work_buffer, int track_len);
BYTE* align_vorpal(BYTE *work_buffer, int track_len);
BYTE* auto_gap(BYTE *work_buffer, int track_len);
BYTE* find_weak_gap(BYTE *work_buffer, int tracklen);
BYTE* find_long_sync(BYTE *work_buffer, int tracklen);
BYTE* auto_gap(BYTE *work_buffer, int tracklen);
