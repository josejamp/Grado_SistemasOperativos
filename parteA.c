#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h> /* for put_user */
#include <asm/io.h>
/*
* Prototypes
*/
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int keyboard_value(const char* buff, size_t len);
static int value_of(char c);


#define SUCCESS 0
#define DEVICE_NAME "parteA" /* Dev name as it appears in /proc/devices */
#define BUF_LEN 80 /* Max length of the message from the device */
#define SCROLL_LOCK 0x01 /* bit 0 */
#define NUM_LOCK 0x02 /* bit 1 */
#define CAPS_LOCK 0x04 /* bit 2 */

/*
* Global variables are declared as static, so are global within the file.
*/
static int Prior_state; /* VARIABLE GLOBAL UTILIZADA SI QUEREMOS QUE EN DOS ESCRITURAS
							DISTNTAS NO SE APAGUEN LOS LEDS YA ENCENDIDOS */
static int Major; /* Major number assigned to our device driver */
static int Device_Open = 0; /* Is device open?
 * Used to prevent multiple access to device */
static char msg[BUF_LEN]; /* The msg the device will give when asked */
static char *msg_Ptr;
static struct file_operations fops = {
	.read =device_read,
	.write =device_write,
	.open = device_open,
	.release = device_release
};
/*
* This function is called when the module is loaded
*/
int init_module(void) {
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	Prior_state = 0;
	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}
	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");
	return SUCCESS;
}
/*
* This function is called when the module is unloaded
*/
void cleanup_module(void) {
	unregister_chrdev(Major, DEVICE_NAME); /* Unregister the device */
}
/*
* Methods
*/
/*
* Called when a process tries to open the device file, like
* "cat /dev/mycharfile"
*/
static int device_open(struct inode *inode, struct file *file) {
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}
/*
* Called when a process closes the device file.
*/
static int device_release(struct inode *inode, struct file *file) {
	Device_Open--;
	/* We're now ready for our next caller */
	/*
	* Decrement the usage count, or else once you opened the file, you'll
	* never get rid of the module.
	*/
	module_put(THIS_MODULE);
	return 0;
}
/*
* Called whenaprocess, which already opened thedevfile, attempts to
* read from it.
*/
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h */
 char *buffer, /* buffer to fill with data */
 size_t length, /* length of the buffer */
 loff_t * offset) {
	
	return -EINVAL;
}
/*
* Called when a process writes to dev file: echo "$num" > /dev/parteA
*/
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off) {
	int retries = 5;
	int timeout = 1000;
	int state = keyboard_value(buff, len); // La configuración de los leds (0x04 por ejemplo)
	//Prior_state = Prior_state | state; //ACTUALIZACIÓN DE LA VARIABLE GLOBAL 
	outb(0xed,0x60); // Le decimos al teclado que queremos modificar los leds
	udelay(timeout);
	while (retries!=0 && inb(0x60)!=0xfa) { // esperamos al controlador
		retries--;
		udelay(timeout);
	}
	if (retries!=0) { // comprobamos que el teclado está listo
		//outb(Prior_state ,0x60); //SOLO SI QUEREMOS QUE NO SE APAGUEN LOS LEDS YA ENCENDIDOS
		outb(state ,0x60);
	}
	return len;
}

static int keyboard_value(const char* buff, size_t len){
	int value = 0x00;
	int i;
	for(i=0; i<len; i++){
		value = value | value_of(buff[i]);
	}

	return value;
}

static int value_of(char c){
	switch(c){
	case '1': return NUM_LOCK; break;
	case '2': return CAPS_LOCK; break;
	case '3': return SCROLL_LOCK; break;
	default: return 0x00; 
	}
}

/* OTRA FORMA DE HACERLO EN VEZ DE LAS DOS FUNCIONES ANTERIORES
	switch(len){
	case(1){
		switch(buff[0]){
		case '1': value = NUM_LOCK; break; //0010
		case '2': value = CAPS_LOCK; break; //0100
		case '3': value = SCROLL_LOCK; break; //0001
		}
	} break;
	case(2){
		if( (buff[0]=='1' && buff[1]=='2') ||
		    (buff[0]=='2' && buff[1]=='1')  ) 
			value = NUM_LOCK | CAPS_LOCK; //0110
		else if( (buff[0]=='1' && buff[1]=='3') ||
			 (buff[0]=='3' && buff[1]=='1')  ) 
			value = NUM_LOCK | SCROLL_LOCK; //0011
		else if( (buff[0]=='2' && buff[1]=='3') ||
			 (buff[0]=='3' && buff[1]=='2')  )
			value = CAPS_LOCK | SCROLL_LOCK; //0011
	} break;
	case(3){
		if( (buff[0]=='1' && buff[1]=='2' && buff[2]=='3') ||
		    (buff[0]=='1' && buff[1]=='3' && buff[2]=='2') ||
		    (buff[0]=='2' && buff[1]=='1' && buff[2]=='3') ||
		    (buff[0]=='2' && buff[1]=='3' && buff[2]=='1') ||
		    (buff[0]=='3' && buff[1]=='1' && buff[2]=='2') ||
		    (buff[0]=='3' && buff[1]=='2' && buff[2]=='1')  ) 
			value = NUM_LOCK | CAPS_LOCK |SCROLL_LOCK;
	} break;
	}
*/
