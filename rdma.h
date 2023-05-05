#include <infiniband/verbs.h>  // RDMA 통신에 필요한 헤더 파일
#include <cstdlib>             // exit() 함수를 사용하기 위해 필요한 헤더 파일
#ifndef __HEADER
#define __HEADER

#define MSG_SIZE 1024          // 메시지 사이즈
#define BUF_SIZE 1024
struct ibv_device **device_list;
struct ibv_device *device;
ibv_context *context; // context : IBV <> kernel module 간 interface 제공
struct ibv_pd *pd;                 // Protection domain
struct ibv_mr *mem_region;                 // 메모리 레지스터(memory region)
struct ibv_cq *comp_que;                 // Completion queue
struct ibv_qp *qp;                 // Queue pair
struct ibv_port_attr port_attr;    // Port attribute
char *msg;                         // 전송할 메시지

// RDMA 통신을 위한 변수들
uint64_t remote_addr;              // 원격지 주소
uint32_t rkey;                     // 원격지 메모리 레지스터 키
int rc;                            // 에러 체크용 변수


#endif