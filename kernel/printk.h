#ifndef INCLUDED_kernel_printk_h
#define INCLUDED_kernel_printk_h

#ifdef __cplusplus
extern "C" {
#endif

#define KERN_SOH     "\001"
#define KERN_EMERG   "0"
#define KERN_WARNING "1"
#define KERN_NOTICE  "2"
#define KERN_INFO    "3"
#define KERN_DEBUG   "4"
#define KERN_DEFAULT "5"

// Integer equivalents of KERN_<LEVEL>
// Inspired by linux kernel
#define LOGLEVEL_EMERG    0
#define LOGLEVEL_WARNING  1
#define LOGLEVEL_NOTICE   2
#define LOGLEVEL_INFO     3
#define LOGLEVEL_DEBUG    4
#define LOGLEVEL_DEFAULT  5

#define pr_emerg(str, ...)   printk(KERN_EMERG str,  ##__VA_ARGS__)
#define pr_warning(str, ...) printk(KERN_WARNING str,##__VA_ARGS__)
#define pr_notice(str, ...)  printk(KERN_NOTICE str, ##__VA_ARGS__)
#define pr_info(str, ...)    printk(KERN_INFO str,   ##__VA_ARGS__)
#define pr_debug(str, ...)   printk(KERN_DEBUG str,  ##__VA_ARGS__)

// printk.c
void            printk(char*, ...);
void            panic(char*) __attribute__((noreturn));
void            printkinit(void);


#ifdef __cplusplus
}
#endif

#endif
