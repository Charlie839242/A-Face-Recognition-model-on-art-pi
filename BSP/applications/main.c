#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "drv_spi_ili9488.h"  // spi lcd driver
#include <lcd_spi_port.h>  // lcd ports
#include <logo.h>
#include <rt_ai.h>
#include <rt_ai_network_model.h>
#include <backend_cubeai.h>

struct rt_event ov2640_event;

char *label[]={"LY","LYC","LZY","Unknown"};

//s摄像头，屏幕相关函数
extern int rt_gc0328c_init(void);
extern void DCMI_Start(void);

//s图像预处理函数
static inline void _itof(float *dst,rt_uint8_t *src, uint32_t size, float div);
void rgb2gray(unsigned char *src,unsigned char *dst, int width,int height);
void bilinera_interpolation(rt_uint8_t* in_array, short height, short width,
                            rt_uint8_t* out_array, short out_height, short out_width);

int main(void)
{

    /* logo 1s showing */
    lcd_show_image(0, 0, 320, 240, LOGO);
    lcd_show_string(90, 140, 16, "Hello RT-Thread!");
    lcd_show_string(90, 180, 16, "Demo: gc0328c & LCD");
    rt_thread_mdelay(1000);

    /* init spi data notify event */
    rt_event_init(&ov2640_event, "ov2640", RT_IPC_FLAG_FIFO);

    /* struct lcd init */
    struct drv_lcd_device *lcd;
    struct rt_device_rect_info rect_info = {0, 0, 320, 240};
    lcd = (struct drv_lcd_device *)rt_device_find("lcd");

    /* ai:获取模型句柄model*/
    rt_ai_t model = NULL;
    model = rt_ai_find(RT_AI_NETWORK_MODEL_NAME);
    if(!model) {rt_kprintf("ai find err\n"); return -1;}

    /* ai:为模型分配运行空间 */
    rt_uint8_t *work_buf = rt_malloc(RT_AI_NETWORK_WORK_BUFFER_BYTES);
    if(!work_buf) {rt_kprintf("malloc err\n");return -1;}

    /* ai:为模型输出分配储存空间 */
    rt_uint8_t *_out = rt_malloc(RT_AI_NETWORK_OUT_1_SIZE_BYTES);
    if(!_out) {rt_kprintf("malloc err\n"); return -1;}

    /* ai:为模型输入分配相应空间 */
    rt_uint8_t *input_buf       = rt_malloc(128*128*sizeof(float));
    if(!input_buf) {rt_kprintf("malloc err\n"); return -1;}

    /* 为图片预处理分配空间 */
    rt_uint8_t *input_gray      = rt_malloc(320*240);                   //s原图转换为灰度图的空间
    rt_uint8_t *input_gray_160  = rt_malloc(128*128);                   //s拉伸后的灰度图的空间

    /* ai:初始化并配置ai模型的输入输出空间 */
    if(rt_ai_init(model,work_buf) != 0){rt_kprintf("ai init err\n"); return -1;}
    rt_ai_config(model,CFG_INPUT_0_ADDR,(rt_uint8_t*)input_buf);
    rt_ai_config(model,CFG_OUTPUT_0_ADDR,_out);



    /* camera init */
    rt_gc0328c_init();
    DCMI_Start();

    rt_kprintf("camera start...\r\n");

    while(1)
    {
        rt_event_recv(&ov2640_event,
                            1,
                            RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                            RT_WAITING_FOREVER,
                            RT_NULL);
        rt_kprintf("receive\n");
        lcd->parent.control(&lcd->parent, RTGRAPHIC_CTRL_RECT_UPDATE, &rect_info);
        //DCMI_Start();  // 显示拍摄到的图像

        /* ai part */
        // 1.s图片预处理
        rgb2gray(lcd->lcd_info.framebuffer ,input_gray, 320,240);                       //s将接收到的图片转化为灰度图并存在input_gray
        bilinera_interpolation(input_gray, 240, 320, input_gray_160, 128, 128);         //s将input_gray中的图片压缩成160*160，并存在input_gray_160
        _itof(input_buf,input_gray_160, 128*128, 255.0);                                //s归一化后存放在input_buf

        // 2.s运行模型并获取结果
        int max_index = 0;
        rt_ai_run(model, NULL, NULL);                                                   //s运行模型
//        rt_kprintf("The max is [%d,%d,%d]\n",_out[0],_out[1],_out[2]);
        rt_kprintf("[ %d , %d , %d , %d ]\n",_out[0],_out[1],_out[2],_out[3]);

        // 3.s处理结果并打印
        for(int i=0 ; i < 4 ; i++)
        {
             if(_out[i] > _out[max_index])
                 max_index = i;
        }                                                                               //s获取最大概率索引

        lcd_show_string(90, 10, 16, label[max_index]);
//        if(_out[max_index]>150)
//            lcd_show_string(90, 10, 16, label[max_index]);
//        else
//            lcd_show_string(90, 10, 16, label[3]);

        DCMI_Start();
    }
    return RT_EOK;
}

#include "stm32h7xx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal QSPI_FLASH */
    SCB->VTOR = QSPI_BASE;
    return 0;
}
INIT_BOARD_EXPORT(vtor_config);


static inline void _itof(float *dst,rt_uint8_t *src, uint32_t size, float div){
    if (div == 0){
        return ;
    }else{
        int i = 0;
        for(i = 0; i < size; i++){
            dst[i] = (float)src[i] / div;
        }
    }
}

// img covnert to gray: Gray = 0.2989*R + 0.5870*G + 0.1140*B
// better: 4898*R + 9618*G + 1868*B >> 14
// int8: 76*R + 150*G + 30*B >> 8
void rgb2gray(unsigned char *src, unsigned char *dst, int width, int height)
{
    int r, g, b;
    for (int i=0; i<width*height; ++i)
    {
        r = *src++; // load red
        g = *src++; // load green
        b = *src++; // load blue
        // build weighted average:
        *dst++ = (r * 76 + g * 150 + b * 30) >> 8;
    }
}

int is_in_array(short x, short y, short height, short width)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
        return 1;
    else
        return 0;
}

void bilinera_interpolation(rt_uint8_t* in_array, short height, short width,
                            rt_uint8_t* out_array, short out_height, short out_width)
{
    double h_times = (double)out_height / (double)height,
           w_times = (double)out_width / (double)width;
    short  x1, y1, x2, y2, f11, f12, f21, f22;
    double x, y;

    for (int i = 0; i < out_height; i++){
        for (int j = 0; j < out_width; j++){
            x = j / w_times;
            y = i / h_times;

            x1 = (short)(x - 1);
            x2 = (short)(x + 1);
            y1 = (short)(y + 1);
            y2 = (short)(y - 1);
            f11 = is_in_array(x1, y1, height, width) ? in_array[y1*width+x1] : 0;
            f12 = is_in_array(x1, y2, height, width) ? in_array[y2*width+x1] : 0;
            f21 = is_in_array(x2, y1, height, width) ? in_array[y1*width+x2] : 0;
            f22 = is_in_array(x2, y2, height, width) ? in_array[y2*width+x2] : 0;
            out_array[i*out_width+j] = (rt_uint8_t)(((f11 * (x2 - x) * (y2 - y)) +
                                       (f21 * (x - x1) * (y2 - y)) +
                                       (f12 * (x2 - x) * (y - y1)) +
                                       (f22 * (x - x1) * (y - y1))) / ((x2 - x1) * (y2 - y1)));
        }
    }
}




