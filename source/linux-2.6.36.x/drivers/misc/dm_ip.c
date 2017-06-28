#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/inet.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/socket.h>
#include <linux/if_ether.h>
#include <asm/checksum.h>


#define DM_IP_ROOT_NAME     "router_domain"
#define DM_IP_ROOT_ENTRY    "dm_ip"
#define DEFAULT_DM_BUFFER   "luyou.cmcc.cn"
#define DEFAULT_IP_BUFFER   "192.168.0.10"
#define DEFAULT_IP          0x0a00a8c0


struct dm_ip_data
{
    struct proc_dir_entry *root;
    struct proc_dir_entry *entry;
    char *dm_buffer;
    char *ip_buffer;
    uint32_t ip;
};


static struct dm_ip_data *dm_ip = NULL;

static int8_t D_name[] =   //这个数组看不懂的话可以去查查DNS应答包字段
{
0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
0x00, 0x64, 0x00, 0x04, 0x0a, 0x0a, 0x0a, 0xfe
};

/**
 * [dm_ip_read description]
 * @param  page  [description]
 * @param  start [description]
 * @param  off   [description]
 * @param  count [description]
 * @param  eof   [description]
 * @param  data  [description]
 * @return       [description]
 */
int dm_ip_read(char *page, char **start, off_t off,int count, int *eof, void *data)
{
    if(NULL == dm_ip)
    {
        printk("Error: dm_ip is NULL\n");
        return 0;
    }
    else
    {
        strcpy(page,dm_ip->dm_buffer);
        strcat(page,"   ");
        strcat(page,dm_ip->ip_buffer);
        strcat(page,"\n");
        return strlen(page);
    }
}

/**
 * [dm_ip_write description]
 * @param  file   [description]
 * @param  buffer [description]
 * @param  count  [description]
 * @param  data   [description]
 * @return        [description]
 */
int dm_ip_write(struct file *file, const char __user *buffer,unsigned long count, void *data)
{
    uint32_t addr;
    int ret = 0;
    if(!strncmp(buffer,"ip",2))
    {
        ret = in4_pton((char*)(buffer + 3),count - 3,(u8*)&addr, '\n', NULL);
        if(1 == ret){
            dm_ip->ip = addr;
            printk(" %s is a valid IPv4 address.addr = %x\n",(char*)(buffer + 3),addr);
        }
        else {
            dm_ip->ip = 0;
            printk(" %s is not a valid IPv4 address,Please check your IP\n",(char*)(buffer + 3));
        }

        ret = copy_from_user(dm_ip->ip_buffer, (char*)(buffer + 3), count - 4);
        dm_ip->ip_buffer[count - 4] = 0;
        ret = count;
    }
    else if(!strncmp(buffer,"dm",2))
    {
        ret = copy_from_user(dm_ip->dm_buffer, (char*)(buffer + 3), count - 4);
        dm_ip->dm_buffer[count - 4] = 0;
        ret = count;
    }
    else
    {
        printk("Your input format is invalid\n");
        printk("Please follow the format below\n");
        printk("echo \"dm www.baidu.com\" \n");
        printk("echo \"ip xxx.xxx.xxx.xxx\" \n");
        ret = -1;
    }

    return ret;
}

/**
 * [create_dm_ip_moudle description]
 * @return  [description]
 */
static int create_dm_ip_moudle(void)
{
    int ret = 0;

    dm_ip = kzalloc(sizeof(struct dm_ip_data),GFP_KERNEL);
    if(NULL == dm_ip)
    {
        printk("dm_ip: request dm_ip size failed\n");
        return -1;
    }
    dm_ip->dm_buffer = kzalloc(128,GFP_KERNEL);
    dm_ip->ip_buffer = kzalloc(32,GFP_KERNEL);
    dm_ip->dm_buffer = DEFAULT_DM_BUFFER;
    dm_ip->ip_buffer = DEFAULT_IP_BUFFER;
    dm_ip->ip = DEFAULT_IP;



    dm_ip->root = proc_mkdir(DM_IP_ROOT_NAME, NULL);
    if(dm_ip->root == NULL)
    {
        printk("create dir router_domain fail\n");
        return -1;
    }

    dm_ip->entry = create_proc_entry(DM_IP_ROOT_ENTRY, 0444, dm_ip->root);
    if(dm_ip->entry==NULL)
    {
        printk("fortune :couldn't create proc entry\n");
        ret = -2;

        return ret;
    }
    dm_ip->entry->read_proc = dm_ip_read;
    dm_ip->entry->write_proc = dm_ip_write;


    return ret;
}

/**
 * [destory_dm_ip_moudle description]
 */
static void destory_dm_ip_moudle(void)
{
    remove_proc_entry(DM_IP_ROOT_ENTRY, dm_ip->root);  
    remove_proc_entry(DM_IP_ROOT_NAME, NULL);
    kfree(dm_ip);
}

/**
 * [ip_check_sum description]
 * @param skb [description]
 */
static void ip_check_sum(struct sk_buff *skb)
{
    struct iphdr *ip = ip_hdr(skb);
    ip->check = 0;
    ip->check = ip_fast_csum((unsigned char *)ip, ip->ihl);
    //printk("ip->check = %x\n",ip->check);
}
/**
 * [udp_checksum description]
 * @param skb [description]
 */
static void udp_checksum(struct sk_buff *skb)
{
    uint32_t cksum = 0;
    struct iphdr *_ip;  
    struct udphdr *_udp;
    uint16_t *p_sum;
    uint16_t  n = 0;
    
    _ip = ip_hdr(skb);
    _udp = (struct udphdr *)(_ip + 1);
    
    //12bytes UDP pseudo header 
    cksum = (_ip->saddr & 0x0000ffff) + (_ip->saddr >> 16) + (_ip->daddr & 0x0000ffff) + (_ip->daddr >> 16);
    cksum += htons(17) + _udp->len;
    cksum = (cksum & 0x0000ffff) + (cksum >> 16);
    
    cksum += _udp->source + _udp->dest + _udp->len + 0;
    
    n = ntohs(_udp->len) - 8;
    p_sum = (uint16_t*)(_udp + 1);
    for(; n > 1; n = n - 2)
    {
        cksum += *p_sum++;
    }
    if(1 == n)
    {
        cksum += *(uint8_t*)p_sum;
    }
    
    cksum = (cksum & 0x0000ffff) + (cksum >> 16);
    cksum = (cksum & 0x0000ffff) + (cksum >> 16); 
    
    _udp->check = ~(cksum & 0x0000ffff);

}

/**
 * [domain_prase description]
 * @param  pos [description]
 * @return     [description]
 */
int domain_prase(uint8_t *pos)
{
    int8_t d_buf[32];
    int8_t i = 0;
    char* temp_buf = d_buf;

    memset(d_buf, 0, sizeof(d_buf));
    for(i = 0; pos[i]; i++)
    {
        if(pos[i] < 20)
            d_buf[i] = '.';
        else
            d_buf[i] = pos[i];
        if(i >= 31)
            return -1;
    }

    if(strnicmp(d_buf,"www.",4) == 0)
        temp_buf += 4;

    if(strnicmp(temp_buf, dm_ip->dm_buffer, strlen(dm_ip->dm_buffer)) == 0)
        return 1;
    else
        return -1;
}



/**
 * [adddata description]
 * @param  skb [description]
 * @return     [description]
 */
int adddata(struct sk_buff * skb)
{
    struct iphdr *ip = NULL;
    struct udphdr *udp = NULL;
    struct ethhdr *ethdr = NULL;
    uint8_t *p = NULL;
    uint16_t *p_data = NULL;
    uint16_t tmpdata = 0;
    uint32_t tmpip = 0;
    uint8_t i = 0 , tmp = 0;

    memcpy(&D_name[sizeof(D_name)-4], &dm_ip->ip, 4); //IP

    p = skb_put(skb, sizeof(D_name));
    if(NULL != p)
        memcpy(p, D_name, sizeof(D_name));

    //
    skb->pkt_type = PACKET_OTHERHOST;
    skb->protocol = __constant_htons(ETH_P_IP);
    skb->ip_summed = CHECKSUM_NONE;
    skb->priority = 0;
    ip = ip_hdr(skb);
    udp = (struct udphdr *)(ip+1);

    //DNS报文修改
    p_data = (uint16_t *)(udp + 1);
    p_data[1] = htons(0x8580); //FLAGS，标准回复
    p_data[3] = htons(1); //AnswerRRs

    //UDP报文修改(校验放到最后一步)
    tmpdata = udp->source; //交换源端口和目的端口
    udp->source = udp->dest;
    udp->dest = tmpdata;
    udp->len = htons(ntohs(udp->len) + sizeof(D_name)); //报文长度

    //IP层修改
    ip->tot_len = htons(ntohs(ip->tot_len) + sizeof(D_name));//报文长度
    ip->id = 0x0000; //IP标识
    ip->frag_off = htons(0x4000);//不分片，无偏移
    tmpip = ip->saddr; //交换源IP和目的IP
    ip->saddr = ip->daddr;
    ip->daddr = tmpip;

    if(skb->mac_header == NULL)
    {
        printk("counterfeit dns data fail! error: skb->mac_header is NULL!\n");
        return -1;
    }

    //以太网头部层
    for(i = 0; i<6; i++)
    {
        tmp = skb->mac_header[i];
        skb->mac_header[i] = skb->mac_header[i+6];
        skb->mac_header[i+6] = tmp;
    }
    
    skb_push(skb , ETH_HLEN); //重要，修改skb->data指针，使其指向MAC头部，并且增加skb->len
    //ok,剩下的就是校验了
    printk("counterfeit DNS success!\n");
    return 1;
}


static unsigned int domain_hook(unsigned int hooknum,struct sk_buff * skb, const struct net_device *in,
    const struct net_device *out, int (*okfn) (struct sk_buff *))
{
    struct iphdr *ip;
    struct udphdr *udp;
    uint8_t *p;

    if (!skb)
        return NF_ACCEPT;


    if(skb->protocol != htons(0x0800)) //排除ARP干扰
        return NF_ACCEPT;

    ip = ip_hdr(skb);
    if(ip->protocol != 17)
        return NF_ACCEPT;

    udp = (struct udphdr *)(ip+1);

    if( (udp != NULL) && (ntohs(udp->dest) != 53) )
    {
        return NF_ACCEPT;
    }
    
    p = (uint8_t*)udp + 21;

    if(domain_prase(p)>0)
    {
        adddata(skb);
        udp_checksum(skb);
        ip_check_sum(skb);
        if(dev_queue_xmit(skb) < 0)
            printk("dev_queue_xmit failed!!\n");
        else
            printk("dev_queue_xmit succeed!!\n");
        return NF_STOLEN;//接管skb
    }
    return NF_ACCEPT;

}

struct nf_hook_ops dm_ip_ops = {
    .list = {NULL, NULL},
    .hook = domain_hook,
    //.pf = PF_INET,
    //.hooknum = NF_INET_PRE_ROUTING,
    //.priority = NF_IP_PRI_FIRST + 1,

    .pf = PF_BRIDGE,
    .hooknum = NF_BR_PRE_ROUTING,
    .priority = NF_BR_PRI_NAT_DST_BRIDGED,
};




static int __init init_dm_ip_moudle(void)
{
    create_dm_ip_moudle();
    nf_register_hook(&dm_ip_ops);
    return 0;
}

static void __exit remove_dm_ip_moudle(void)
{
    destory_dm_ip_moudle();
    nf_unregister_hook(&dm_ip_ops);
}

module_init(init_dm_ip_moudle);
module_exit(remove_dm_ip_moudle);


MODULE_AUTHOR("liaowenjie <liaowenjie@cmiot.chinamobile.cn>");
MODULE_DESCRIPTION("dm_ip module");
MODULE_LICENSE("GPL");