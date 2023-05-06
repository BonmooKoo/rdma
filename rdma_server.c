#include "rdma.h"

int main(){
    device_list=ibv_get_device_list(NULL);
    if(!device_list){
        printf("No IB device\n");
        exit(1);
    }
    device=device_list[0];
    if(!device){
        printf("No IB device\n");
        exit(1);
    }
    context=ibv_open_device(device);
    if(!context){
        printf("No Context device\n");
        exit(1);
    }
    pd=ibv_alloc_pd(context);
    if(!pd){
        printf("No protection domain\n");
        exit(1);
    }
    msg=(char*)malloc(MSG_SIZE);
    mem_region=ibv_reg_mr(pd,msg,MSG_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);

    if(!mem_region){
        printf("No mem region\n");
        exit(1);
    }
//Complete Queue 생성
    comp_que=ibv_create_cq(context,10,NULL,NULL,0);
    //                     contect, cqe cq_context , complete channel , comp_vector
    if(!comp_que){
        printf("No comp_que\n");
        exit(1);
    }

    struct ibv_qp_init_attr qp_init_attr={};
    memset(&qp_init_attr,0,sizeof(qp_init_attr));
    qp_init_attr.qp_type=IBV_QPT_RC; // queue pair type ; reliable connection
    qp_init_attr.send_cq=comp_que;
    qp_init_attr.recv_cq=comp_que;
    qp_init_attr.cap.max_send_wr=32; // Work Request
    qp_init_attr.cap.max_recv_wr=32;
    qp_init_attr.cap.max_send_sge=1; //Scatter gather entry
    qp_init_attr.cap.max_recv_sge=1;


//Queue pair 생성
    qp=ibv_create_qp(pd,&qp_init_attr);
    if(!qp){
        printf("No queue pair\n");
        exit(1);
    }
     // QP 상태 변경
    struct ibv_qp_attr conn_attr={};
    memset(&conn_attr,0,sizeof(conn_attr));
    conn_attr.qp_state = IBV_QPS_INIT;
    conn_attr.path_mtu=IBV_MTU_256;
    conn_attr.pkey_index = 0;
    conn_attr.port_num = 1;
    conn_attr.qp_access_flags = IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE;
    if (ibv_modify_qp(qp, &conn_attr, IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS)) {
        fprintf(stderr, "Failed to modify QP state to INIT: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // QP 연결 대기
    memset(&conn_attr, 0, sizeof(conn_attr));
    conn_attr.qp_state = IBV_QPS_RTR;
    conn_attr.path_mtu = IBV_MTU_256;
    conn_attr.dest_qp_num = remote_qpn;
    conn_attr.rq_psn = remote_psn;
    conn_attr.max_dest_rd_atomic = 1;
    conn_attr.min_rnr_timer = 12;
    conn_attr.ah_attr.is_global = 0;
    conn_attr.ah_attr.dlid = remote_lid;
    conn_attr.ah_attr.sl = 0;
    conn_attr.ah_attr.src_path_bits = 0;
    conn_attr.ah_attr.port_num = 1;
    if (ibv_modify_qp(qp, &conn_attr, IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
                    IBV_QP_DEST_QPN | IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC |
                    IBV_QP_MIN_RNR_TIMER)) {
        fprintf(stderr, "Failed to modify QP state to RTR: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    //QP 연결 완료
    memset(&conn_attr, 0, sizeof(conn_attr));
    conn_attr.qp_state = IBV_QPS_RTS;
    conn_attr.sq_psn = local_psn;
    conn_attr.timeout = 14;
    conn_attr.retry_cnt = 7;
    conn_attr.rnr_retry = 7;
    conn_attr.max_rd_atomic = 1;
    if (ibv_modify_qp(qp, &conn_attr, IBV_QP_STATE | IBV_QP_SQ_PSN | IBV_QP_TIMEOUT |
                    IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY | IBV_QP_MAX_QP_RD_ATOMIC)) {
        fprintf(stderr, "Failed to modify QP state to RTS: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

        // 클라이언트가 보낸 문자열 받기
    if (ibv_post_recv(qp, &r_wr, &bad_r_wr)) {
        fprintf(stderr, "Failed to post receive: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 버퍼에서 문자열 읽기
    if (ibv_poll_cq(cq, 1, &wc) < 1) {
        fprintf(stderr, "Failed to poll receive CQ: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (wc.status != IBV_WC_SUCCESS) {
        fprintf(stderr, "Failed to receive message: %s\n", ibv_wc_status_str(wc.status));
        exit(EXIT_FAILURE);
    }

    // 받은 문자열 출력하기
    printf("Received message from client: %s\n", recv_buf);

}

