/* ASSIGNMENT 2 : INFORMATION ASSURING BLOCK DRIVER
 * CS519
 * AUTHORS : MANDANNA TK , KARAN DESAI
 * 
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/kthread.h>  /* for the kThread */
#include <linux/sched.h>   /* Task for Kernel Thread */

MODULE_LICENSE("Dual BSD/GPL");
static char *Version = "1.4";
static char* name;                                    //path of block device to be monitored

static u64 start_sector = 0;
static u64 range_sector=0;
static int major_num;

module_param(start_sector, int, 0);                  //starting block from where monitoring is performed
module_param(range_sector,int,0);		     //range of blocks to be monitored
static int logical_block_size = 512;
//module_param(logical_block_size, int, 0);
static int nsectors = 1024; /* How big the drive is */
//module_param(nsectors, int, 0);

module_param(name,charp,S_IRUGO);
/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */

#define KERNEL_SECTOR_SIZE 512

/*
 * Our request queue.
 */


/*
 * The internal representation of our device.
 */
static struct my_device {
	unsigned long size;
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
	struct block_device * blkdev;
	char * name;
	struct request_queue *Queue;
} Device;

struct bio *temp_bio;

struct task_struct *task; // A task for the kernel Thread

/*
 * Handle an I/O request.
 */
static void my_transfer(struct my_device *dev, sector_t sector,unsigned long nsect, char *buffer, int write)
{
	unsigned long offset = sector * logical_block_size;                 //obtaining offset in terms of bytes
	unsigned long nbytes = nsect * logical_block_size;
	unsigned long begin_sector = start_sector * logical_block_size;
	unsigned long range = range_sector * logical_block_size;

	
	char *temp_buffer1;
	//char *temp_buffer2;

	int diff = range-nbytes;

	if(diff < 0)
	diff=diff*(-1);

	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "my: Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}
	if (write)
	{	
			memcpy(temp_buffer1, dev->data + offset, nbytes);		//read old data from device and compare
		
			if(strcmp(temp_buffer1,buffer))
			{
				printk(KERN_ALERT "\n Content has been changed\n ");
			}
		memcpy(dev->data + offset, buffer, nbytes);
		
	}
	else
		memcpy(buffer, dev->data + offset, nbytes);
}

/*
 * The HDIO_GETGEO ioctl is handled in blkdev_ioctl(), which
 * calls this. We need to implement getgeo, since we can't
 * use tools such as fdisk to partition the drive otherwise.
 */

int my_getgeo(struct block_device * block_device, struct hd_geometry * geo)
 {
	long size;

	/* We have no real geometry, of course, so make something up. */
	size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	return 0;
}


static int my_bd_xfer_bio(struct my_device *dev, struct bio *bio)
{
	int i;
	struct bio_vec *bvec;
	sector_t sector = bio->bi_sector;
	// Do each segment independently. 
	bio_for_each_segment(bvec, bio, i) {
	
    char *buffer = __bio_kmap_atomic(bio, i, KM_USER0);                     //ensuring that bio is in high memory
	
    my_transfer(&Device, sector, (bvec->bv_len >> KERNEL_SECTOR_SIZE),buffer, bio_data_dir(bio));  //==WRITE
	sector += (bvec->bv_len >> KERNEL_SECTOR_SIZE);         //bio_cur_sectors(bio);
	__bio_kunmap_atomic(bio, KM_USER0);
	
    }
	
    return 0; // Always "succeed"
}


void bookkeeping(struct bio* bio)
{
	int status;

	struct bio * temp_bio=bio_clone(bio,GFP_KERNEL);   //cloning the received bio and performing a copy of the same transfer on our virtual device
	
	status = my_bd_xfer_bio(&Device, temp_bio);
	
}

static void bd_make_request(struct request_queue *q, struct bio *bio)
{
	//struct bd_dev *dev = q->queuedata;
	int status=0;
	//status = bd_xfer_bio(dev, bio);
	bio->bi_bdev= Device.blkdev;


	/* temp_bio=bio_clone(bio,GFP_KERNEL);
	status = my_bd_xfer_bio(&Device, bio);
	bookkeeping(bio);
	perform bookkeeping before calling this function 
    */
     
    task = kthread_run(&bookkeeping,bio,"Kthread");
    
    printk(KERN_INFO "Kernel Thread : %s\n",task->comm);
    
	submit_bio(bio_data_dir(bio), bio);
	
	bio_endio(bio,status);
	//return 0;
}

/*
 * The device operations structure.
 */
static struct block_device_operations my_ops = {
		.owner  = THIS_MODULE,
		.getgeo = my_getgeo
};

static int __init my_init(void) {
	/*
	 * Set up our internal device.
	 */
	Device.size = nsectors * logical_block_size;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if (Device.data == NULL)
		return -ENOMEM;
	/*
	 * Get a request queue.
	 */
	Device.blkdev=blkdev_get_by_path(name,FMODE_READ|FMODE_WRITE|FMODE_EXCL,&Device);

	if(Device.blkdev==NULL)
	{
		printk(KERN_ALERT "\n did not get block device for %s \n",name);
		goto out;
	}
	else
	{
		printk(KERN_ALERT "\n device name is %s \n",name);
	}
	
	Device.Queue = bdev_get_queue(Device.blkdev);
	printk(KERN_ALERT "entering if condition");
	Device.Queue = blk_alloc_queue(GFP_KERNEL);
	if (Device.Queue == NULL)
	goto out;
	blk_queue_make_request(Device.Queue, bd_make_request);

	/*
	 * Get registered.
	 */
	major_num = register_blkdev(major_num, "my");                    //our virtual device
	if (major_num < 0) {
		printk(KERN_WARNING "my: unable to get major number\n");
		goto out;
	}
	/*
	 * And the gendisk structure.
	 */
	Device.gd = alloc_disk(16);
	if (!Device.gd)
		goto out_unregister;
	Device.gd->major = major_num;
	Device.gd->first_minor = 0;
	Device.gd->fops = &my_ops;
	Device.gd->private_data = &Device;
	strcpy(Device.gd->disk_name, "my0");
	set_capacity(Device.gd, nsectors);
	Device.gd->queue = Device.Queue;
	add_disk(Device.gd);

	return 0;

out_unregister:
	unregister_blkdev(major_num, "my");
out:
	vfree(Device.data);
	return -ENOMEM;
}

static void __exit my_exit(void)
{
	
    kthread_stop(task);
    del_gendisk(Device.gd);
	put_disk(Device.gd);
	unregister_blkdev(major_num, "my");
	blk_cleanup_queue(Device.Queue);
	vfree(Device.data);
}

module_init(my_init);
module_exit(my_exit);
