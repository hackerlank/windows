####  第 5 章  Windows内存管理

  Windows 规定运行在用户态(Ring3层)的程序，只能访问用户模式地址,而运行在核心态(Ring0层)的程序，可以访问整个4G的虚拟内存。

####  分页和非分页内存
  
  Windows规定有些虚拟内存页面是可以交换到文件中的，这类内存被称为分页内存。
  而有些虚拟内存页永远不会交换到文件中，这些内存被称为非分页内存。

  当程序的中断请求级在DISPATCH_LEVEL之上时(包括DISPATCH_LEVEL层),程序只能使用非分页内存，否则将导致蓝屏死机。

    #pragma  PAGEDCODE
    VOID  SomeFunction()
    {
        PAGE_CODE();
        // 做一些事情
    }

    PAGE_CODE()是DDK提供的宏,它只在check版本中生效。它会检验这个函数是否运行低于DISPATCH_LEVEL的中断请求级，如果等于或高于这个中断请求级,会产生一个断言。

####  分配内核内存

    PVOID  ExAllocatePool(IN POOL_TYPE PoolType, IN SIZE_T NumberOfBytes);


    PVOID  ExAllocatePoolWithTag(IN POOL_TYPE PoolType,
                                 IN SIZE_T NumberOfBytes, IN ULONG Tag);



    PVOID  ExAllocatePoolWithQuota(IN POOL_TYPE PoolType, 
                                   IN SIZE_T NumberOfBytes);




    PVOID  ExAllocatePoolWithQuotaTag(IN POOL_TYPE PoolType,
                                      IN SIZE_T NumberOfBytes, IN ULONG Tag);

    PoolType:是一个枚举变量，如果此值为NonPagedPool,则分配非分页内存。
             如果此值为PagedPool，则分配内存为分页内存。
    NumberOfBytes:是分配内存的大小,注意最好是4的整倍。

    返回值:分配的内存地址，一定是内核模式地址。如果返回0，则代表分配失败。

    函数以WithQuota结尾的代表分配的时候按配额分配。
    函数以WithTag结尾的函数，和ExAllocatePool功能类似，唯一不同的是多了一个Tag参数，系统在要求的内存外又额外地分配了4个字节的标签。在调试的时候，可以找出是否有标有这个标签的内存没有被释放。

    以上4个函数都需要指定PoolType，分别可以指定如下几种:

    NonPagedPool:要求分配非分页内存

    PagedPool:分配分页内存

    NonPagedPoolMustSucceed:分配非分页内存，必须成功

    DontUseThisType:未指定

    NonPagedPoolCacheAligned:分配非分页内存，而且必须内存对齐

    PagedPoolCacheAligned:分配分页内存，而且必须内存对其

    NonPagedPoolCacheAlignedMusrS:指定分配非分页内存，而且必须内存对其，而且必须成功.


  将分配的内存进行回收
  
    #define ExFreePool(a) ExFreePoolWithTag(a,0)

    VOID  ExFreePool(IN PVOID P);

    VOID  ExFreePoolWithTag(IN PVOID P, IN ULONG Tag);

    参数P就是要释放的地址

####  驱动中使用链表

    typedef struct _LIST_ENTRY {
        struct _LIST_ENTRY *Flink;
        struct _LIST_ENTRY *Blink;
    } LIST_ENTRY, *PLIST_ENTRY, *RESTRICTED_POINTER PRLIST_ENTRY;

    DDK提供了标准的双向链表。双向链表可以将链表形成一个环。Blink指针指向前一个元素，Flink指针指向下一个元素。

####  链表初始化

  每个双向链表都是以一个链表头作为链表的第一个元素。初次使用链表需要进行初始化，主要将链表头的Flink和Blink两个指针都指向自己。
  这意味着链表头所代表的是空链。初始化链表头用InitializeListHead宏实现

    VOID InitializeListHead(OUT PLIST_ENTRY ListHead)
    {
        ListHead->Flink = ListHead->Blink = ListHead;
    }
  
  检查链表是否为空:使用IsListEmpty宏
    BOOLEAN IsListEmpty(IN const LIST_ENTRY * ListHead)
    {
        return (BOOLEAN)(ListHead->Flink == ListHead);
    }


####  从首部插入链表

    VOID InsertHeadList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
    {

        PLIST_ENTRY Flink;

        Flink = ListHead->Flink;
        Entry->Flink = Flink;
        Entry->Blink = ListHead;
        Flink->Blink = Entry;
        ListHead->Flink = Entry;
    }

    例:InsertHeadList(&head, &mydata->ListEntry)
    mydata是用户自定义的数据结构，而它的子域ListEntry是包含其中的LIST_ENTRY数据结构。

####  从尾部插入链表

    VOID InsertTailList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry)
    {
        PLIST_ENTRY Blink;

        Blink = ListHead->Blink;
        Entry->Flink = ListHead;
        Entry->Blink = Blink;
        Blink->Flink = Entry;
        ListHead->Blink = Entry;
    }


####  链表删除

  `1` 头删除 

    PLIST_ENTRY RemoveHeadList(PLIST_ENTRY ListHead)
    {
        PLIST_ENTRY Flink;
        PLIST_ENTRY Entry;

        Entry = ListHead->Flink;
        Flink = Entry->Flink;
        ListHead->Flink = Flink;
        Flink->Blink = ListHead;
        return Entry;
    }


  `2` 尾删除

    PLIST_ENTRY RemoveTailList(PLIST_ENTRY ListHead)
    {

        PLIST_ENTRY Blink;
        PLIST_ENTRY Entry;

        Entry = ListHead->Blink;
        Blink = Entry->Blink;
        ListHead->Blink = Blink;
        Blink->Flink = ListHead;
        return Entry;
    }

    PLIST_ENTRY pEntry = RemoveTailList(&head);
    其中，head是链表头，pEntry是从链表删除下来的元素中的ListEntry。这里有一个问题，就是如何从pEntry得到用户自定义数据结构指针。

  `1` 当自定义的数据结构的第一个字段是LIST_ENTRY时

    此时，RemoveTailList返回的指针可以当做用户自定义的指针。即
    PLIST_ENTRY pEntry = RemoveTailList(&head);
    PMYDATA pMydata = (PMYDATA)pEntry;

  `2` 当用户自定义数据结构的第一个字段不是LIST_ENTRY时。

    typedef struct _MYDATA
    {
        ULONG x;
        ULONG y;
        LIST_ENTRY ListEntry;
    }MYDATA, *PMYDATA;


    此时，RemoveTailList返回的指针不可以当做用户自定义的指针。

    可以使用CONTAINING_RECORD宏得到用户数据指针。
    PLIST_ENTRY pEntry = RemoveTailList(&head);
    PIRP pIrp = CONTAINING_RECORD(pEntry, MYDATA, ListEntry);



####  Lookaside结构

####  运行时函数

`1` 内存间复制(非重叠)
  
    #define RtlCopyMemory(Destination,Source,Length) /
            memcpy((Destination),(Source),(Length))

    Length:表示要复制内存的长度，单位字节

`2` 内存间复制(可重叠)

    #define RtlMoveMemory(Destination,Source,Length) \
            memmove((Destination),(Source),(Length))

`3` 填充内存

    #define RtlFillMemory(Destination,Length,Fill) 
            memset((Destination),(Fill),(Length))

    #define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

`4` 内存比价
    
    #define RtlEqualMemory(Destination,Source,Length)
            (!memcmp((Destination),(Source),(Length)))

`5`









