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
    comp_que=ibv_create_cq(context,1,NULL,NULL,0);
    if(!comp_que){
        printf("No comp_que\n");
        exit(1);
    }

    struct ibv_qp_init_attr qp_init_attr={};
    memset(&qp_init_attr,0,sizeof(qp_init_attr));
    qp_init_attr.send_cq=comp_que;
    qp_init_attr.recv_cq=comp_que;
    qp_init_attr.qp_type=IBV_QPT_RC; // queue pair type ; reliable connection
    qp_init_attr.cap.max_send_wr=1; // Work Request
    qp_init_attr.cap.max_recv_wr=10;
    qp_init_attr.cap.max_send_sge=1; //Scatter gather entry
    qp_init_attr.cap.max_recv_sge=1;


//Queue pair 생성
    qp=ibv_create_qp(pd,&qp_init_attr);
    if(!qp){
        printf("No queue pair\n");
        exit(1);
    }
// QP State 변경 
// INIT > RTR / RTS
// 수신만하는 쪽은 RTR도 ok
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
    }//conn_attr로 설정한 내용을 qp에 저장
    printf("QP %d : RTS state\n", qp->qp_num);
/////////////////////////////////////////////////

//송신버퍼 / 수신 버퍼 생성하고 QP에 등록하기
//두개의 Queue pair을 통신 가능한 상태로 설정하기

    char* send_buf = (char *) malloc(BUF_SIZE);
    char* recv_buf = (char *) malloc(BUF_SIZE);
    memset(send_buf, 0, BUF_SIZE);
    memset(recv_buf, 0, BUF_SIZE);

    // 송신, 수신 버퍼 메모리 등록
    struct ibv_mr* send_mr = ibv_reg_mr(pd, send_buf, BUF_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
    struct ibv_mr* recv_mr = ibv_reg_mr(pd, recv_buf, BUF_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);

    uintptr_t remote_addr=(uintptr_t) recv_buf;
    //remote_key=recv_mr->rkey;

    //버퍼에 데이터 쓰기
    sprintf(send_buf,"Hello RDMA! From Konduck1\n");

    struct ibv_send_wr send_wr,*fail_send_wr;
    struct ibv_sge sge;
    memset(&sge, 0, sizeof(sge));
    memset(&send_wr,0,sizeof(send_wr));
    // sge / send_wr 설정하기
    sge.addr = (uintptr_t)send_buf; //sge의 주소 설정하기
    sge.length = BUF_SIZE;
    sge.lkey = mem_region->lkey; //memory region의 lkey 설정하기
    send_wr.wr_id = (uint64_t)(uintptr_t)sge.addr;
    send_wr.opcode = IBV_WR_SEND_WITH_IMM;
    send_wr.sg_list = &sge;
    send_wr.num_sge = 1;
    send_wr.imm_data = htonl(123456789); //임의의 32비트 값
    ibv_post_send(qp,&send_wr,&fail_send_wr); //send work request 전송


    printf("Success : Send request posted\n");

    struct ibv_wc wc;//Work Complete
    while(1){
        int net = ibv_poll_cq(comp_que,1,&wc);
        if(net<0){
            printf("Fail to Poll CQ\n");
            exit(1);
        }
        if(net=0){
            continue;
        }
        if(wc.status!=IBV_WC_SUCCESS){
            printf("Failed for wr_id %d\n",wc.wr_id);
            exit(1);
        }
    }

    ibv_destroy_qp(qp);
    ibv_destroy_cq(comp_que);
    ibv_dealloc_pd(pd);
    ibv_close_device(context);
    ibv_free_device_list(device_list);
    free(msg);

    return 0;
}
