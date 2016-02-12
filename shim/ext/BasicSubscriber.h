#ifdef __cplusplus
extern "C"
{
#endif
    void unsubscribe ( );
    int subscribe ( );
    int poll( char* message, int n );
#ifdef __cplusplus
}
#endif
