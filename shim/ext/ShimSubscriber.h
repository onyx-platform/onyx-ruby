#ifdef __cplusplus
extern "C"
{
#endif
    void unsubscribe ( );
    int subscribe( char* channel, int streamId );
    int poll( char* message, int n );
#ifdef __cplusplus
}
#endif
