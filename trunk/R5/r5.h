#ifndef R5
#define R5

//Sybolic Constants
#define PIC_MASK 0x21

#define OPEN 900
#define CLOSED 901

//Derror Codes
#define INV_EVT_FLAG      -101;
#define INV_BAUD_RATE     -102;
#define INV_PORT_CLOSED   -103;

typedef struct {
      int flag;
      int* evt_flag;
      
      int status;
      
      //Address and counters current input buffer
      char* in_buff;
      int *in_count;
      int in_done;
      
      //address and coutners current output buffer
      char* out_buff;
      int *out_count;
      int out_done;
      
      //Ring buffer members
      char[size] ring_buffer;  
      int ring_buffer_in;  
      int ring_buffer_out;
      int ring_buffer_count;
    
}dcb;


/*****************************
 *Prototypes
 */
 
int com_open(int eflag_p, int baud_rate);
int com_close(void);
int com_read(char* buf_p, int* count_p);
int com_write(char* buf_p, int* count_p);

//DCB Prototypes
int Allocate_DCB();
int Setup_DCB();


//Interrupts
void interrupt sys_call_1();      //Level 1
void read_int();                  //Level 2
void write_int();                 //Level 2
 
//Main functions 
void init();
void clear_screen();

//Error
void errorTrans(int);

#endif