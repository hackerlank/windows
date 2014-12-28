#### 第 6 章  Windows内核函数


######  ASCII字符串和宽字符串
  
  两种字符串:

  `1` 一种是char型的字符串，负责记录ANSI字符集，它是指向一个char数组的指针，每个char型变量的大小为一个字节，以0标志字符串结束。

  `2` 另一种是wchar_t型的宽字符串，负责描述Unicode字符集的字符串，它是指向一个wchar_t数组的指针，wchar_t字符大小为两个字节，字符串以0标志结束。

  ANSI字符的构造如下:

    char *str1 = "abc";    str1指针指向的内容是 61 62 63 00 

  UNICODE字符的构造如下:

    wchar_t *str2 = L"abc";   str2指针指向的内容是 6100 6200 6300 0000; 构造字符串的时候使用一个关键字"L",编译器会自动生成所需要的宽字符.

  DDK中:
  `1` char-->CHAR
  `2` wchar_t --> WCHAR

  打印ASCII字符串:
    CHAR *string = “Hello";
    KdPrint(("%s\n", string));   小写%s

  打印宽字符串:
    WCHAR *string = L"hello";
    KdPrint(("%S\n", string));   大写%S

######  ANSI_STRING 字符串 与 UNICODE_STRING 字符串

	typedef struct _STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		PCHAR Buffer;
	}STRING;
	typedef STRING *PSTRING;
	typedef STRING ANSI_STRING;
	typedef PSTRING PANSI_STRING;

	typedef STRING OEM_STRING;
	typedef PSTRING POEM_STRING;
	typedef CONST STRING* PCOEM_STRING;

	这个数据结构对ASCII字符串进行了封装:
	`1` Length:字符的长度
	`2` MaximumLength:整个字符串缓冲区的最大长度
	`3` Buffer:缓冲区的指针

	注意:和标准的字符串不同，STRING字符串不是以0标志字符串的结束,字符长度依靠Length字段。



	与ANSI_STRING相对应,DDK将宽字符串封装成UNICODE_STRING数据结构:

	typedef WCHAR *PWCHAR, *LPWCH, *PWCH;

	typedef struct _UNICODE_STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		PWCH Buffer;
	}
	typedef UNICODE_STRING *PUNICODE_STRING;
	typedef const UNICODE_STRING *PCUNICODE_STRING;

	`1` Length：字符串长度，单位字节。如果是N个字符，那么Length等于N的2被
	`2` MaximumLength：整个字符缓冲区的最大长度，单位字节
	`3` Buffer：缓冲区的指针

	和ANSI_STRING不同，UNICODE_STRING的缓冲区是记录宽字符的缓冲区。每个元素是宽字符。
	和ANSI_STRING一样，字符串的结束不是以NULL为标志，而是依靠字段Length

###### 字符初始化与销毁


  ANSI_STRING字符串和UNICODE_STRING字符串使用前需要进行初始化。有两种办法构造这个数据结构。

  `1` 使用DDK提供的相关函数

  初始化ANSI_STRING字符串:

    VOID RtlInitAnsiString(IN OUT PANSI_STRING DestinationString, IN PCSZ SourceString);

    1. DestinationString: 要初始化的ANSI_STRING字符串;
    2. SourceString: 字符串的内容

    VOID RtlInitUnicodeString(IN OUT PUNICODE_STRING DestinationString, IN PCWSTR SourceString);

    1. DestinationString: 要初始化的UNICODE_STRING字符串;
    2. SourceString: 字符串的内容
    例如:

    VOID InitSting()
	{
		ANSI_STRING AnsiString;
		UNICODE_STRING UnicodeString;

		CHAR *AString = "AnsiString hello";
		WCHAR *UString = L"Unicode_string hello";

		RtlInitAnsiString(&AnsiString, AString);
		RtlInitUnicodeString(&UnicodeString, UString);
		KdPrint(("ANSI_STRING --> %Z\n", &AnsiString));
		KdPrint(("UNICODE_STRING --> %Z\n", &UnicodeString));
	}

	这种办法是将AnsiString中的Buffer指针等于AString指针。这种初始化的优点是操作简单，用完后不用清理内存。但带来另外一个问题 如果修改AString，同时会导致AnsiString字符发生改变:


  `2` 另外一种方法是程序员自己申请内存，并初始化内存，当不用字符串时,需要回收字符串占用的内存

  
