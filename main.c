#include <stdio.h>
#include <string.h>
#include "ztimer.h"
#include "shell.h"
#include "thread.h"
#include "xtimer.h"
#include "net/nanocoap.h"
#include "net/nanocoap_sock.h"
#include "net/sock/udp.h"
#include "net/ipv6/addr.h"
#include "saul.h"
#include "saul_reg.h"
#include "phydat.h"

void printTemperature(int raw, int scale);

static phydat_t temp;

void *_temp_scanner(void *arg){
    (void) arg;
    while(true){
        saul_reg_t *temp_sensor_dev = saul_reg_find_type(SAUL_SENSE_TEMP);
        if (temp_sensor_dev == NULL) {
            printf("Temperature sensor not found!\n");
        }
        if (temp_sensor_dev->driver == NULL) {
            printf("Temperature sensor driver is not available!\n");
        }
        if (saul_reg_read(temp_sensor_dev, &temp) < 0) {
            printf("Error reading temperature sensor\n");
        } else {
            printf("Raw temperature value: %ld, scale: %d\n", (long)temp.val[0], temp.scale);
            printTemperature((long)temp.val[0], temp.scale);
        }
        ztimer_sleep(ZTIMER_SEC, 300);
    }
}

static ssize_t hello_handler(coap_pkt_t *pkt, uint8_t *buf, size_t len,
                               coap_request_ctx_t *ctx)
{
    (void)ctx;
    char payload[128];
    snprintf(payload, sizeof(payload), "Temperature: %ld and Scale: %d", (long)temp.val[0], temp.scale);
    return coap_reply_simple(pkt, COAP_CODE_CONTENT, buf, len,
                             COAP_FORMAT_TEXT, (uint8_t *)payload, strlen(payload));
}

NANOCOAP_RESOURCE(temperature) {
    .path    = "/temperature",
    .methods = COAP_GET,
    .handler = hello_handler,
};

#define COAP_INBUF_SIZE (256U)
#define MAIN_QUEUE_SIZE (8)

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
static uint8_t buf[COAP_INBUF_SIZE];


void *_event_loop(void *arg) {
    printf("Event loop baby\n");
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    ztimer_sleep(ZTIMER_MSEC, 3000);
    
    (void)arg;
    
    sock_udp_ep_t local = {
        .port   = COAP_PORT,
        .family = AF_INET6
    };
    
    nanocoap_server(&local, buf, sizeof(buf));
    
    while(1) {
        thread_yield();
    }
    return NULL;
}

void printTemperature(int raw, int scale) {
    if (scale < 0) {
        int divisor = 1;
        for (int i = 0; i < -scale; i++) {
            divisor *= 10;
        }
        int whole = raw / divisor;
        int decimal = raw % divisor;
        printf("Temperature: %d.%0*d °C\n", whole, -scale, decimal);
    } else {
        printf("Temperature: %ld x 10^%d °C\n", (long)raw, scale);
    }
}

int main(void)
{
    puts("Simple RIOT nanocoAP server example");
    printf("{\"IPv6 addresses\": [\"");
    netifs_print_ipv6("\", \"");
    puts("\"]}");

    static char event_loop_stack[THREAD_STACKSIZE_LARGE];

    thread_create(
        event_loop_stack,
        sizeof(event_loop_stack),
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_WOUT_YIELD,
        _event_loop,
        NULL,
        "event_loop_thread"
   );


   static char scanner_stack[THREAD_STACKSIZE_DEFAULT];

   thread_create(
       scanner_stack,
       sizeof(scanner_stack),
       THREAD_PRIORITY_MAIN - 2,
       THREAD_CREATE_WOUT_YIELD,
       _temp_scanner,
       NULL,
       "temp_scanner"
  );

   char line_buf[SHELL_DEFAULT_BUFSIZE];
   shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
   return 0;
}
