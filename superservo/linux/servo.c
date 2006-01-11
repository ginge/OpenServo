#define __KERNEL__
#define MODULE
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <linux/timex.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/types.h>

#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>


#include "servo.h"
#include "sysdep.h"

//----------------------------------------------------------------------
// notes


//----------------------------------------------------------------------
// constants, globals 

#define MOD_NAME ("servo")
#define MOD_MAJOR (140)
#define MOD_SCRATCH_SIZE 4096

//----------------------------------------------------------------------
// module info

MODULE_AUTHOR( "Colin MacKenzie (guru@colinmackenzie.net)" );
MODULE_DESCRIPTION( "Open source i2c driver for digital servo." );
MODULE_LICENSE("GPL");


//----------------------------------------------------------------------
// i2c client globals

unsigned short normal_i2c[] = {0x10, I2C_CLIENT_END };
unsigned short normal_i2c_range[] = { /* 0x10, 0x20, */ I2C_CLIENT_END };

I2C_CLIENT_INSMOD;



//----------------------------------------------------------------------
// prototypes

// module init, remove
int init_module();
void cleanup_module();

// driver prototypes
void servo_i2c_inc_use(struct i2c_client *driv);
void servo_i2c_dec_use (struct i2c_client *driv);
int servo_attach_adapter(struct i2c_adapter* adap);
int servo_detach_client(struct i2c_client* client);
int servo_command(struct i2c_client* client, unsigned int cmd, void* arg);


// driver callbacks
int servo_onfinddevice(struct i2c_adapter* adap, int addr, unsigned short flags, int kind);



// client prototypes



//----------------------------------------------------------------------
// servo defines

/* Dual Operation Commands */
#define SERVO_RW_REGISTERS	0x01
#define SERVO_RW_REGION		0x02
#define SERVO_DEVICE_INFO       0x0a
#define SERVO_ECHO_COMMAND      0x0d
#define SERVO_RESET             0x0e


/* Single Operation Commands */
#define SERVO_WRITE_REGISTERS	0x11
#define SERVO_WRITE_REGION	0x12
#define SERVO_SET_POSITION	0x14
#define SERVO_SET_SPEED		0x15
#define SERVO_ENABLE		0x16
#define SERVO_DISABLE		0x17
#define SERVO_RESTORE_SETTINGS	0x18
#define SERVO_LOAD_SETTINGS	0x19
#define SERVO_SAVE_SETTINGS	0x1a
#define SERVO_RESET		0x0e
#define SERVO_PROGRAMMING	0x0f





//----------------------------------------------------------------------
// global structures

struct i2c_client servo_i2c_client = {
  name    : "Digital Servo",
  id      : 0,
  flags   : I2C_CLIENT_ALLOW_USE,
  addr    : 0,
  adapter : NULL,
  driver  : NULL,
  data    : NULL,
  usage_count : 0
};

struct servo_i2c_client_data {
	/* proc interfaces for servo */
	struct proc_dir_entry* proc_servo;
	struct proc_dir_entry* proc_info;
	struct proc_dir_entry* proc_position;
	struct proc_dir_entry* proc_speed;
	struct proc_dir_entry* proc_enable;
	struct proc_dir_entry* proc_status;
	struct proc_dir_entry* proc_pid;

	/* information retrieved from servo */
	char name[32];
	char copyright[32];
	char version[8];
	char revision[2];
	char cpu[32];
	char algorithm[32];
};

struct i2c_driver servo_i2c_driver = {
  name    : "Digital Servo Driver",
  id      : I2C_DRIVERID_EXP0,
  flags   : I2C_DF_NOTIFY,
  attach_adapter  : servo_attach_adapter,
  detach_client   : servo_detach_client,
  command         : servo_command,
  inc_use : servo_i2c_inc_use,
  dec_use : servo_i2c_dec_use,
};


//----------------------------------------------------------------------
// global variables

/* main servo proc directory */
struct proc_dir_entry* proc_servos;


//----------------------------------------------------------------------
// implementation

const char* hex = "0123456789abcdef";

static int servo_proc_read_position( char* page, char** start, off_t offset, int count, int* eof, void* v_client) {
  int len, err;
  unsigned char msg[10];
  struct i2c_client* client = (struct i2c_client*)v_client;

  /* send read region command */
  msg[0]=SERVO_RW_REGION;
  msg[1]=0x60;
  if( (err=i2c_master_send(client, msg, 2)) <0 ) {
	printk( KERN_INFO "servo_read: failed to send readRegion command to device %02x.\n", client->addr );
	*eof=1;
	return 0;
  }
    
  /* read servo position */
  if( (err=i2c_master_recv(client, msg, 2)) <0 ) {
	printk( KERN_INFO "failed to read position from servo %02x.\n", client->addr );
	*eof=1;
	return 0;
  }
  
  len=0;
  page[len++] = hex[ msg[1]/16 ];
  page[len++] = hex[ msg[1]%16 ];
  page[len++] = ' ';
  page[len++] = hex[ msg[0]/16 ];
  page[len++] = hex[ msg[0]%16 ];
  page[len++] = '\n';
  page[len++] = 0;
  
  return len;
}

static int servo_proc_write_position( struct file* file, const char* buffer, unsigned long count, void* v_client) {
  int err, x=0;
  char* c;
  unsigned char msg[10];
  struct i2c_client* client = (struct i2c_client*)v_client;

  if(count>=2) {
    /* convert hex value to int */
    if( (c=strchr(hex, buffer[0])) == NULL)
	    return count;
    x = (c-hex)*16;

    if( (c=strchr(hex, buffer[1])) == NULL)
	    return count;
    x += (c-hex);
    
    /* set position */
    msg[0]=SERVO_SET_POSITION;
    msg[1]=x;
    if( (err=i2c_master_send(client, msg, 2)) <0 )
	    printk( KERN_INFO "failed to set position for device %02x.\n", client->addr );
  }
  return count;
}

static int servo_proc_read_info( char* page, char** start, off_t offset, int count, int* eof, void* v_client) {
	char* out = page;
	struct i2c_client* client = (struct i2c_client*)v_client;
	struct servo_i2c_client_data* data = (struct servo_i2c_client_data*)client->data;

	/* define a macro to output our members */
#define OUT(field) strcpy(out, #field ": "); out+=strlen( #field )+2; strcpy(out, data->field); out+=strlen(data->field); *out='\n'; out++;
	OUT(name);
	OUT(copyright);
	OUT(version);
	OUT(revision);
	OUT(cpu);
	OUT(algorithm);
#undef OUT

	return out-page;
}

static int servo_proc_read_speed( char* page, char** start, off_t offset, int count, int* eof, void* v_client) {
  int len, err;
  unsigned char msg[10];
  struct i2c_client* client = (struct i2c_client*)v_client;

  /* send read register command */
  msg[0]=SERVO_RW_REGISTERS;
  msg[1]=0x62;
  if( (err=i2c_master_send(client, msg, 2)) <0 )
	printk( KERN_INFO "servo_read: failed to send readRegion command to device %02x.\n", client->addr );

  /* read servo position */
  if( (err=i2c_master_recv(client, msg, 1)) <0 )
	printk( KERN_INFO "failed to transmit to servo device %02x.\n", client->addr );

  len=0;
  page[len++] = hex[ msg[0]/16 ];
  page[len++] = hex[ msg[0]%16 ];
  page[len++] = '\n';
  page[len++] = 0;
  return len;
}

static int servo_proc_write_speed( struct file* file, const char* buffer, unsigned long count, void* v_client) {
  int err, x=0;
  char* c;
  unsigned char msg[10];
  struct i2c_client* client = (struct i2c_client*)v_client;

  if(count>=2) {
    /* convert hex value to int */
    if( (c=strchr(hex, buffer[0])) == NULL)
            return count;
    x = (c-hex)*16;

    if( (c=strchr(hex, buffer[1])) == NULL)
            return count;
    x += (c-hex);

    /* set position */
    msg[0]=SERVO_SET_SPEED;
    msg[1]=x;
    if( (err=i2c_master_send(client, msg, 2)) <0 )
            printk( KERN_INFO "failed to set speed for device %02x.\n", client->addr );
  }
  return count;
}

static int servo_proc_read_enable( char* page, char** start, off_t offset, int count, int* eof, void* v_client) {
  int len, err;
  unsigned char msg[10];
  struct i2c_client* client = (struct i2c_client*)v_client;

  /* send read region command */
  msg[0]=SERVO_RW_REGISTERS;
  msg[1]=0x64;
  if( (err=i2c_master_send(client, msg, 2)) <0 )
	  printk( KERN_INFO "servo_read: failed to send readRegion command to device %02x.\n", client->addr );

  /* read servo position */
  if( (err=i2c_master_recv(client, msg, 1)) <0 )
	  printk( KERN_INFO "failed to transmit to servo device %02x.\n", client->addr );

  len=0;
  page[len++] = (msg[0] & 0x04) ? '1' : '0';
  page[len++] = '\n';
  page[len++] = 0;
  return len;
}

static int servo_proc_write_enable( struct file* file, const char* buffer, unsigned long count, void* v_client) {
  int err;
  unsigned char msg[10];
  struct i2c_client* client = (struct i2c_client*)v_client;
  
  if(count>=2) {
	if(buffer[0]=='1')
	  msg[0] = SERVO_ENABLE;
	else if(buffer[0]=='0')
	  msg[0] = SERVO_DISABLE;
	else
	  return 1;

	/* set position */
	if( (err=i2c_master_send(client, msg, 1)) <0 )
		printk( KERN_INFO "failed to set position for device %02x.\n", client->addr );
  }
  return count;
}

static int servo_proc_read_status( char* page, char** start, off_t offset, int count, int* eof, void* v_client) {
  int len, err;
  unsigned char msg[10];
  struct i2c_client* client = (struct i2c_client*)v_client;

  /* send read region command */
  msg[0]=SERVO_RW_REGISTERS;
  msg[1]=0x64;
  if( (err=i2c_master_send(client, msg, 2)) <0 )
	    printk( KERN_INFO "servo_read: failed to send readRegion command to device %02x.\n", client->addr );

  /* read servo position */
  if( (err=i2c_master_recv(client, msg, 1)) <0 )
	    printk( KERN_INFO "failed to transmit to servo device %02x.\n", client->addr );

  len=0;
  page[len++] = hex[ msg[0]/16 ];
  page[len++] = hex[ msg[0]%16 ];
  page[len++] = '\n';
  page[len++] = 0;
  return len;
}

static int servo_proc_read_pid( char* page, char** start, off_t offset, int count, int* eof, void* v_client) {
  int len, err, i;
  unsigned char msg[32];
  struct i2c_client* client = (struct i2c_client*)v_client;

  /* send read region command */
  msg[0]=SERVO_RW_REGION;
  msg[1]=0x20;
  if( (err=i2c_master_send(client, msg, 2)) <0 )
	    printk( KERN_INFO "servo_read: failed to send readRegion command to device %02x.\n", client->addr );

  /* read servo position */
  if( (err=i2c_master_recv(client, msg, 8)) <0 )
	    printk( KERN_INFO "failed to transmit to servo device %02x.\n", client->addr );

  strcpy(page, "kp   ki   kd   kpid \n");
  len=21;
  for(i=0; i<4; i++) {
    page[len++] = hex[ msg[i*2]/16 ];
    page[len++] = hex[ msg[i*2]%16 ];
    page[len++] = hex[ msg[i*2+1]/16 ];
    page[len++] = hex[ msg[i*2+1]%16 ];
    page[len++] = ' ';
  }
  page[len++] = '\n';
  page[len++] = 0;
  return len;
}





int parse_device_info( struct servo_i2c_client_data* data, const char* deviceInfo) {
	int field, sz;
	const char *l, *r;

	for(r=l=deviceInfo+1, field=0; (*(r-1)!=0) && (field<6); r++) {
		if((*r=='\n') || (*r==0)) {
			/* next field */
			switch(field) {
				case 0: sz = ((r-l)<sizeof(data->name)) ? r-l : sizeof(data->name)-1;
					strncpy(data->name, l, sz); 
					data->name[ sz ] = 0; 
					break;
				case 1: sz = ((r-l)<sizeof(data->copyright)) ? r-l : sizeof(data->copyright)-1;
					strncpy(data->copyright, l, sz); 
					data->copyright[ sz ] = 0; 
					break;
				case 2: sz = ((r-l)<sizeof(data->version)) ? r-l : sizeof(data->version)-1;
					strncpy(data->version, l, sz); 
					data->version[ sz ] = 0;
					break;
				case 3: sz = ((r-l)<sizeof(data->revision)) ? r-l : sizeof(data->revision)-1;
					strncpy(data->revision, l, sz); 
					data->revision[ sz ] = 0;
					break;
				case 4: sz = ((r-l)<sizeof(data->cpu)) ? r-l : sizeof(data->cpu)-1;
					strncpy(data->cpu, l, sz); 
					data->cpu[ sz ] = 0;
					break;
				case 5: sz = ((r-l)<sizeof(data->algorithm)) ? r-l : sizeof(data->algorithm)-1;
					strncpy(data->algorithm, l, sz); 
					data->algorithm[ sz ] = 0;
					break;
			};
			field++;
			l=r+1;
		}
	}

	return 0;
}


void servo_i2c_inc_use (struct i2c_client *driv) {
#ifdef MODULE
      MOD_INC_USE_COUNT;
#endif
}

void servo_i2c_dec_use (struct i2c_client *driv) {
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}

/* This function serves as a callback for each device found on an adapter bus
 * during probing. */
int servo_onfinddevice(struct i2c_adapter* adap, int addr, unsigned short flags, int kind) {
  char name[255], s[1024];
  int err;

  /* declare client structues */
  struct i2c_client* client;
  struct servo_i2c_client_data* data;
  
  /* allocate client structure */
  client = (struct i2c_client*)kmalloc( sizeof(struct i2c_client), GFP_KERNEL );

  /* initialize client structures */
  *client = servo_i2c_client;

  printk( KERN_INFO "found device at address %02x.\n", addr);
  
  /* set new client members */
  client->addr = addr;
  client->adapter = adap;
  client->driver = &servo_i2c_driver;

  /* ensure device is a super servo */
  memset(name, 0, sizeof(name) );
  name[0]=SERVO_DEVICE_INFO;
	// send device info query command
  if( (err=i2c_master_send(client, name, 1)) <0 )
	printk( KERN_INFO "failed to transmit device info query request to device %02x.\n", client->addr );
  	// get device info
  else if( (err=i2c_master_recv(client, name, 11)) <0 )
  	printk( KERN_INFO "failed to query device info length from device %02x.\n", client->addr );
  else {
	if( strcmp( name+1, "SuperServo" )!=0 ) {
		printk( KERN_INFO "device %02x does not appear to be a servo, (no device info).\n", client->addr );
		return -1;
	}
	if( (err=i2c_master_recv(client, name, (int)name[0]+1)) <0 ) {
		printk( KERN_INFO "failed to query device info from device %02x.\n", client->addr );
		return -1;
	}
  }

  
  /* create client user data */
  data = (struct servo_i2c_client_data*)kmalloc( sizeof(struct servo_i2c_client_data), GFP_KERNEL );
  client->data = data;

  /* parse device info */
  parse_device_info(data, name);
  printk( KERN_INFO " deviceInfo('%s')\n", name );
  printk( KERN_INFO " device name = '%s'.\n", data->name );
  printk( KERN_INFO " device copyright = '%s'.\n", data->copyright );
  printk( KERN_INFO " device version = '%s'.\n", data->version );
  printk( KERN_INFO " device revision = '%s'.\n", data->revision );
  printk( KERN_INFO " device cpu = '%s'.\n", data->cpu );
  printk( KERN_INFO " device algorithm = '%s'.\n", data->algorithm );
  
  /* get adapter bus id */
  if( (err=i2c_adapter_id( client->adapter )) <0 ) {
	  printk( KERN_INFO "failed to get adapter bus id for '%s'.\n", client->adapter->name );
	  return 0;
  }
  
  /* build proc entry name */
  name[0]='b';
  name[1]=hex[ err/16 ];
  name[2]=hex[ err%16 ];
  name[3]='-';
  name[4]='s';
  name[5]=hex[ client->addr/16 ];
  name[6]=hex[ client->addr%16 ];
  name[7]=0;
  
  /* create servo proc entry directory */
  strcpy(s, "sys/dev/servos/");
  strcat(s, name );
  data->proc_servo = proc_mkdir( s, NULL );
  if( !proc_servos ) {
	printk( KERN_INFO "failed to create servo proc interface directory.\n");
	return -1;
  }
  data->proc_servo->owner = THIS_MODULE;
	
  
  /* define a macro for defining the proc entries */
#define SERVO_PROC_READ_ENTRY(name) { \
          data->proc_##name = create_proc_read_entry( #name, 644, data->proc_servo, servo_proc_read_##name, client); \
          if( !data->proc_##name ) \
            printk( KERN_INFO "failed to create servo proc entry for servo %02x.\n", client->addr ); \
          data->proc_##name->owner = THIS_MODULE; \
        }
#define SERVO_PROC_ENTRY(name) { \
  	  data->proc_##name = create_proc_entry( #name, 644, data->proc_servo ); \
	  if( !data->proc_##name ) \
	    printk( KERN_INFO "failed to create servo proc entry for servo %02x.\n", client->addr ); \
	  else { \
	    data->proc_##name->data = client; \
	    data->proc_##name->read_proc = servo_proc_read_##name; \
	    data->proc_##name->write_proc = servo_proc_write_##name; \
	    data->proc_##name->owner = THIS_MODULE; \
	  } \
	}

  /* define out proc entries */
  SERVO_PROC_READ_ENTRY(info);
  SERVO_PROC_ENTRY(position);
  SERVO_PROC_ENTRY(speed);
  SERVO_PROC_ENTRY(enable);
  SERVO_PROC_READ_ENTRY(status);
  SERVO_PROC_READ_ENTRY(pid);

  /* undefine our proc entries macro */
#undef SERVO_PROC_ENTRY
#undef SERVO_PROC_READ_ENTRY
  
  /* add client */
  i2c_attach_client(client);
  
  return 0;
}

int servo_attach_adapter(struct i2c_adapter* adap) {
  int err;
  
  printk( KERN_INFO "New adapter attached: '%s', probing devices.\n", adap->name );

  if( (err=i2c_probe( adap, &addr_data, servo_onfinddevice)) <0 ) 
	  printk( KERN_INFO "Failed to probe devices on '%s'.\n", adap->name );
  	
  return 0;
}

int servo_detach_client(struct i2c_client* client) {
  int err;
  struct servo_i2c_client_data* data = (struct servo_i2c_client_data*)client->data;

  printk( KERN_INFO "Detaching client at address %02x.\n", client->addr );

  /* detach the client from i2c */
  if( (err=i2c_detach_client( client )) <0 )
	  printk( KERN_INFO "failed to detach digital servo client at address %02x.\n", client->addr );
 
  /* define a macro to remove client proc entries */
#define REMOVE_SERVO_PROC_ENTRY(procname) \
    remove_proc_entry( data->proc_##procname->name, data->proc_servo );
    
  /* remove client proc entries */
  REMOVE_SERVO_PROC_ENTRY(info);
  REMOVE_SERVO_PROC_ENTRY(position);
  REMOVE_SERVO_PROC_ENTRY(speed);
  REMOVE_SERVO_PROC_ENTRY(enable);
  REMOVE_SERVO_PROC_ENTRY(status);
  REMOVE_SERVO_PROC_ENTRY(pid);

  remove_proc_entry( data->proc_servo->name, proc_servos );

  /* undefine our temp macro */
#undef REMOVE_SERVO_PROC_ENTRY
  
  /* free client data */
  if( client->data )
	  kfree( client->data );
  
  /* free client struct */
  kfree(client);
	
  return 0;
}

int servo_command(struct i2c_client* client, unsigned int cmd, void* arg) {

  return 0;
}

const char* errorstr(int e) {
#define CASE(x) case x: return #x;
	switch(abs(e)) {
		CASE( ETIMEDOUT )
		CASE( ENODEV )
		CASE( EREMOTEIO )
		CASE( EFAULT )
	default:
		return "E_UNKNOWN";
	}
#undef CASE
}






// install module into running kernel
int init_module() 
{ 
  int err = 0;

  printk( KERN_DEBUG "Loading servo driver.\n" );

  /* create main proc directory in /proc/sys/dev */
  proc_servos = proc_mkdir( "sys/dev/servos", NULL );
  if( !proc_servos ) {
	printk( KERN_INFO "failed to create servos proc interface.\n");
  	return -1;
  }
  proc_servos->owner = THIS_MODULE;
  

  /* add servo i2c driver */
  if( (err=i2c_add_driver( &servo_i2c_driver )) <0 )
	  printk( KERN_INFO "Failed to add servo driver, error %d.\n", err );
  
  
  return 0;
}

// remove module from running kernel
void cleanup_module() 
{
  int err;
	
  /* unregister i2c client driver */
  if( (err=i2c_del_driver( &servo_i2c_driver )) <0 )
	  printk( KERN_INFO "Failed to delete servo driver, error %d.\n", err );

  /* unregister proc interfaces */
  remove_proc_entry( "sys/dev/servos", NULL );
}

