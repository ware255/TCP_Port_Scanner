#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cerrno>

#define PS_CONNECT    0     /* ポートに接続できた場合 */
#define PS_NOCONNECT  1     /* ポートに接続できなかった場合 */
#define PS_ERROR      2     /* 接続エラー */

/* グローバル変数 */
int rc, sock;

/*
 * タイムアウトを設定したソケットを作成する
 * 受信設定をしないと正常にタイムアウトしない。(仕様？)
 */
int create_timeout_socket()
{
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        return -1;
    }
    
    /* 送信タイムアウトを設定する */
    struct timeval send_tv;
    send_tv.tv_sec  = 1;/* 1秒に設定 */
    send_tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &send_tv, sizeof(send_tv));
    
    /* 受信タイムアウトを設定する */
    struct timeval recv_tv;
    recv_tv.tv_sec = 5;/* 5秒に設定 */
    recv_tv.tv_sec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recv_tv, sizeof(recv_tv));
    
    return sock;
}

/*
 * 接続できるか確認する
 */
int check_connect(int* socks, struct sockaddr *dest, size_t dest_size)
{
    /* socksがNULLではないか確認(ポインタ渡しの場合) */
	  if(socks == NULL){
        perror("Error");
        exit(1);
    }
    
    errno = 0;
    /* 接続だぁぁぁあ！ */
    rc = connect(*socks, dest, dest_size);
    
    /*接続できたお(＾＾) */
    if(rc == 0){
        return PS_CONNECT;
    }
    
    /* 接続出来なかったお(；；) */
    if(errno == ECONNREFUSED){
        return PS_NOCONNECT;
    }
    else if(errno != 0){
        perror("connect");
        return PS_ERROR;
    }
    
    return PS_NOCONNECT;
}

/*
 * 指定ポートに接続を試みる
 */
int connect_to_port(char *ipaddr, int &n_port)
{
    struct sockaddr_in dest;
    
    if((sock = create_timeout_socket()) < 0){
        return PS_ERROR;
    }
    
    /* 接続するための下準備 */
    memset(&dest, 0, sizeof(dest));
    dest.sin_family      = AF_INET;
    dest.sin_port        = htons(n_port);
    dest.sin_addr.s_addr = inet_addr(ipaddr);
    
    /* 接続出来るかチェック */
    rc = check_connect(&sock, (struct sockaddr *)&dest, sizeof(dest));
    
    /* ソケットを閉じる */
    close(sock);
    
    return rc;
}

/*
 * ポート番号に対応するサービス名を出力するだけ
 */
void print_portname(int& port_num)
{
    struct servent *se;
    
    if((se = getservbyport(htons(port_num), "tcp")) == NULL){
        fprintf(stdout, "%d/tcp, unknown\n", port_num);
    }
    else {
        fprintf(stdout, "%d/tcp, %s\n", port_num, se->s_name);
    }
}

/*
 * IPアドレス表記を変換 && ポートスキャン
 */
void ports_scan(char *ipaddr, int start_port, int end_port)
{
    /* ただの関数ポインタ。ちょっとした小細工ですね。*/
    int(*ctp)(char*, int&) = &connect_to_port;
    
    for(int port_num{start_port}; port_num < end_port; port_num++){
        /* ポートスキャン実行 */
        rc = (*ctp)(ipaddr, port_num);
        
        /* エラー時には強制終了する */
        if(rc == PS_ERROR){
            exit(1);
        }
        
        /* 接続できればポート情報を出力する */
        if(rc == PS_CONNECT){
            print_portname(port_num);
        }
    }
}

/*
 * IPアドレス表記を変換 && ポートスキャン
 * 一つのポート番号のみ調査
 */
void port_scan(char *ipaddr, int &start_port)
{
    /* ただの関数ポインタ。ちょっとした小細工ですね。*/
    int(*ctp)(char*, int&) = &connect_to_port;
    
    /* ポートスキャン実行 */
    rc = (*ctp)(ipaddr, start_port);
    
    /* エラー時には強制終了する */
    if(rc == PS_ERROR){
        exit(1);
    }
    
    /* 接続できればポート情報を出力する */
    if(rc == PS_CONNECT){
        print_portname(start_port);
    }
    else {
        printf("%d/tcp, Not open.\n", start_port);
    }
}

/*
 * IPアドレスの名前解決を行う。
 */
int resolve_ipaddr(char *ipaddr)
{
    struct hostent *host;
    struct in_addr addr;
    
    /* 文字列表現のIPアドレスをバイナリ値に変換する */
    addr.s_addr = inet_addr(ipaddr);
    
    /* ホスト名かIPアドレスの調査からIPアドレスの取得 */
    if (inet_aton(ipaddr, &addr) == 0){
        if((host = gethostbyname(ipaddr)) == NULL){
            return 1;
        }
    }
    else {
        if((host = gethostbyaddr((const char *)&addr.s_addr, sizeof(addr.s_addr), AF_INET)) == NULL){
            return 1;
        }
    }
    
    return 0;
}

/*
 * main関数
 * 注意:自作関数とはちょっとだけ仕様が異なります。
 */
int main(int argc, char *argv[])
{
    /* 入力方法の説明 */
    if(argc <= 2){
        fprintf(stdout, "Usage: %s <ip address> <mode: --fast, --all, --port [number]>\n", argv[0]);
        return 1;/* エラーの場合、カーネルに1を返す */
    }
    
    /* 接続先を確認する */
    if((rc = resolve_ipaddr(argv[1])) != 0){
        fprintf(stderr, "Error: Failed to connect to %s\n", argv[1]);
        return 1;
    }
    
    /* --fast と --all で分岐します */
    if(strcmp(argv[2], "--fast") == 0){
        /* ポートは1番から5000番まで指定してスキャン */
        printf("Scanning for %s\n\n", argv[1]);
        ports_scan(argv[1], 1, 1024);
        printf("\nEnd.\n");
    }
    else if(strcmp(argv[2], "--all") == 0){
        /* ポートは1番から65535番まで指定してスキャン */
        printf("Scanning for %s\n\n", argv[1]);
        ports_scan(argv[1], 1, 65535);
        printf("\nEnd.\n");
    }
    else if(strcmp(argv[2], "--port") == 0 && argc >= 3){
        int port_num;
        try {
            port_num = (atoi(argv[3]) <=  1 && atoi(argv[3]) >=  65535) ? 65535 : 1;
        }
        catch (...) {
            printf("Error: Enter the appropriate value.\n");
            return 1;
        }
        printf("Scanning for %s\n\n", argv[1]);
        port_scan(argv[1], port_num);
        printf("\nEnd.\n");
    }
    else {/* 上二つの条件に当てはまらかった場合の分岐 */
        printf("Error: Please specify the options properly.\n");
        return 1;
    }
    
    return 0;/* 正常に終了できたならカーネルに0を返す */
}