/*
 * cdrill.c — C 语法刷题训练程序
 * 编译：clang -std=c11 -Wall -Wextra -O2 cdrill.c -o cdrill
 * 运行：./cdrill
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#define MAX_Q 64
#define MAX_TOPICS 16

typedef enum { TYPE_MC, TYPE_NUM } QType;

typedef struct {
    const char *topic;
    QType type;
    const char *stem;
    const char *opts[4];
    int mc_answer;
    long num_answer;
    const char *explain;
} Question;

#define MC(topic, stem, o0, o1, o2, o3, ans, explain) \
    { topic, TYPE_MC, stem, { o0, o1, o2, o3 }, ans, 0L, explain }
#define NUM(topic, stem, ans, explain) \
    { topic, TYPE_NUM, stem, { NULL, NULL, NULL, NULL }, 0, (long)(ans), explain }

static const Question bank[] = {
    /* ===== 数据类型与运算 ===== */
    NUM("数据类型与运算",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", -7 / 2);",
        -3,
        "C99 起整数除法向零截断：-3.5 变成 -3，不是 -4（向下取整是 Python 的行为）。"),
    NUM("数据类型与运算",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", -7 % 2);",
        -1,
        "取模结果的符号跟随被除数：-7 % 2 = -1。"),
    NUM("数据类型与运算",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", (int)7.9);",
        7,
        "强制类型转换是截断小数部分，不是四舍五入。要四舍五入得用 floor(x + 0.5)。"),
    NUM("数据类型与运算",
        "下面程序的输出是什么？（char 默认为有符号）\n"
        "    char c = 200;\n"
        "    printf(\"%d\\n\", c);",
        -56,
        "200 超出 signed char 的 0~127 范围，按 8 位补码解释就是 200 - 256 = -56。\n"
        "  存进 char 之前一定要确认取值范围；想要 0~255 请用 unsigned char。"),
    NUM("数据类型与运算",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", 0.1 + 0.2 == 0.3);",
        0,
        "0.1 和 0.2 在二进制浮点里都无法精确表示，相加得到 0.30000000000000004。\n"
        "  比较浮点数要用 fabs(a - b) < 1e-6，不要直接 ==。"),
    NUM("数据类型与运算",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", 'A' + 1);",
        66,
        "字符常量本质是 int，'A' 的值是 65，加 1 得 66。"),
    NUM("数据类型与运算",
        "下面程序的输出是什么？\n"
        "    enum { RED = 1, GREEN, BLUE };\n"
        "    printf(\"%d\\n\", BLUE);",
        3,
        "枚举值从上一个值继续递增：RED=1、GREEN=2、BLUE=3。"),
    MC("数据类型与运算",
       "下面程序的输出是什么？\n"
       "    if (-1 > 1U)\n"
       "        puts(\"true\");\n"
       "    else\n"
       "        puts(\"false\");",
       "true", "false", "编译错误", "未定义行为", 0,
       "int 与 unsigned 混合运算时，int 会被转换成 unsigned：-1 变成 UINT_MAX，\n"
       "  所以 -1 > 1U 成立。clang 的 -Wsign-compare 能提前提示这类问题。"),

    /* ===== 运算符与优先级 ===== */
    NUM("运算符与优先级",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", 7 / 2 * 2);",
        6,
        "先算 7 / 2（整数除法得 3），再乘 2 得 6。整数除法丢掉的余数找不回来。"),
    NUM("运算符与优先级",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", 2 << 3);",
        16,
        "左移 3 位等于乘 2 的 3 次方：2 * 8 = 16。"),
    NUM("运算符与优先级",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", 1 | 2 ^ 3);",
        1,
        "异或 ^ 的优先级高于按位或 |，先算 2 ^ 3 = 1，再 1 | 1 = 1。\n"
        "  记不住就加括号：1 | (2 ^ 3)。"),
    NUM("运算符与优先级",
        "x 为 5，下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", x & (x - 1));",
        4,
        "x - 1 把最低位的 1 变成 0、后面的 0 全变 1，再与 x 做与运算就清掉了最低位的 1。\n"
        "  常用于判断 x 是否为 2 的幂：x & (x - 1) == 0。"),
    MC("运算符与优先级",
       "下面程序的输出是什么？\n"
       "    int a = 1;\n"
       "    if (a = 0)\n"
       "        puts(\"T\");\n"
       "    else\n"
       "        puts(\"F\");",
       "T", "F", "编译错误", "未定义行为", 1,
       "a = 0 是赋值不是比较：把 a 改成 0 后条件为假，输出 F。\n"
       "  编译器通常给出 -Wparentheses 警告，见到它就要警惕 == 写成了 =。"),
    NUM("运算符与优先级",
        "a=1, b=2, c=3，下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", a + b > c ? b : a);",
        1,
        "关系运算符 > 的优先级高于条件运算符 ?:，先判断 a + b > c 即 3 > 3，为假，取 a 得 1。"),
    NUM("运算符与优先级",
        "下面程序的输出是什么？\n"
        "    int v = (1, 2, 3);\n"
        "    printf(\"%d\\n\", v);",
        3,
        "逗号表达式从左到右依次求值，整个表达式的值是最后一个，所以 v = 3。"),
    MC("运算符与优先级",
       "下面程序的输出是什么？\n"
       "    int a = 5;\n"
       "    int b = a--;\n"
       "    printf(\"%d %d\\n\", a, b);",
       "4 5", "5 4", "5 5", "4 4", 0,
       "a-- 是后缀自减：先把旧值 5 赋给 b，再让 a 减 1。\n"
       "  如果是 --a，则先减再赋值，结果会是 4 4。"),
    MC("运算符与优先级",
       "下面哪一条语句是未定义行为（UB）？",
       "int i = 5; int j = i++ + 1;",
       "int i = 5; int j = i++ + i++;",
       "int i = 5; i = i + 1;",
       "int i = 5; int j = (i++, i);", 1,
       "两个 i++ 之间没有序列点，同一个变量在一轮求值中被修改两次，标准规定为未定义行为。\n"
       "  其余三条都有确定的求值顺序：逗号运算符和分号都是序列点。"),
    NUM("运算符与优先级",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", 10 > 5 ? 10 > 8 ? 8 : 5 : 0);",
        8,
        "条件运算符 ?: 是右结合的，等价于 10 > 5 ? (10 > 8 ? 8 : 5) : 0。\n"
        "  10 > 5 为真，再看内层 10 > 8 为真，取 8。"),

    /* ===== 控制流 ===== */
    NUM("控制流",
        "下面程序的输出是什么？\n"
        "    int s = 0;\n"
        "    for (int i = 1; i <= 5; i += 2)\n"
        "        s += i;\n"
        "    printf(\"%d\\n\", s);",
        9,
        "i 依次取 1、3、5，累加得 1 + 3 + 5 = 9。步长是 2，不是 1。"),
    MC("控制流",
       "x 为 2，下面程序的输出是什么？\n"
       "    switch (x) {\n"
       "    case 1: printf(\"A\");\n"
       "    case 2: printf(\"B\");\n"
       "    case 3: printf(\"C\"); break;\n"
       "    default: printf(\"D\");\n"
       "    }",
       "B", "BC", "C", "BCD", 1,
       "从匹配的 case 2 开始执行，遇到 break 才跳出，所以先输出 B 再顺着输出 C。\n"
       "  case 1 不匹配，不会执行。这就是所谓的 fallthrough，忘了 break 是常见 bug。"),
    NUM("控制流",
        "下面程序的输出是什么？\n"
        "    int i = 0, n = 0;\n"
        "    while (i < 5) {\n"
        "        i++;\n"
        "        if (i == 3)\n"
        "            continue;\n"
        "        n += i;\n"
        "    }\n"
        "    printf(\"%d\\n\", n);",
        12,
        "i 为 3 时执行 continue 跳过累加，其余累加：1 + 2 + 4 + 5 = 12。\n"
        "  continue 只是跳过本次循环体剩余部分，不跳过 i++（这里 i++ 在 continue 之前）。"),
    NUM("控制流",
        "下面程序的输出是什么？\n"
        "    int i = 0;\n"
        "    while (i++ < 3)\n"
        "        ;\n"
        "    printf(\"%d\\n\", i);",
        4,
        "后置自增在比较之后才生效：最后一次比较时 i 为 3，比较后 i 变成 4，循环结束。"),
    NUM("控制流",
        "i 为 0，下面程序的输出是什么？\n"
        "    int r = (i != 0) && (10 / i > 2);\n"
        "    printf(\"%d\\n\", r);",
        0,
        "&& 短路求值：左边 i != 0 为假，右边根本不计算，所以不会除以 0，r 为 0。\n"
        "  利用短路做空指针/除零保护是 C 的惯用法。"),
    NUM("控制流",
        "下面程序的输出是什么？\n"
        "    int i = 0;\n"
        "    do {\n"
        "        i += 2;\n"
        "    } while (i < 7);\n"
        "    printf(\"%d\\n\", i);",
        8,
        "do-while 先执行后判断：i 依次为 2、4、6、8。i 为 6 时 6 < 7 仍成立，再执行一次得到 8，\n"
        "  此时 8 < 7 为假退出。"),

    /* ===== 数组与字符串 ===== */
    NUM("数组与字符串",
        "下面程序的输出是什么？\n"
        "    int a[5] = { 1, 2 };\n"
        "    printf(\"%d\\n\", a[4]);",
        0,
        "部分初始化时，没有写到的元素自动置 0。局部数组如果完全没初始化，值是不确定的。"),
    MC("数组与字符串",
       "下面程序的输出是什么？\n"
       "    printf(\"%d %d\\n\", (int)strlen(\"hello\"), (int)sizeof(\"hello\"));",
       "5 和 6", "6 和 6", "5 和 5", "6 和 5", 0,
       "strlen 数到 '\\0' 之前，是 5；sizeof 统计整个数组，包含结尾的 '\\0'，是 6。"),
    NUM("数组与字符串",
        "下面程序的输出是什么？\n"
        "    int m[2][3] = { { 1 }, { 4 } };\n"
        "    printf(\"%d\\n\", m[1][2]);",
        0,
        "二维数组按行初始化，每行没写到的元素都是 0：m[1][2] = 0。\n"
        "  注意 m[1] 是第二行，下标从 0 开始。"),
    NUM("数组与字符串",
        "下面程序的输出是什么？\n"
        "    printf(\"%d\\n\", strcmp(\"apple\", \"Apple\") < 0);",
        0,
        "strcmp 按字符编码逐位比较：'a'(97) > 'A'(65)，返回一个正数，所以 < 0 为假，输出 0。\n"
        "  需要忽略大小写请用 strcasecmp。"),
    MC("数组与字符串",
       "下面程序的运行结果是什么？\n"
       "    char *p = \"abc\";\n"
       "    p[0] = 'A';\n"
       "    puts(p);",
       "输出 Abc", "未定义行为（通常崩溃）", "输出 abc", "编译错误", 1,
       "字符串字面量存放在只读数据段，修改它是未定义行为，运行时常见表现为段错误。\n"
       "  要修改就写成 char s[] = \"abc\"; 数组版在自己栈上有可写的副本。"),
    NUM("数组与字符串",
        "下面程序的输出是什么？\n"
        "    int a[6] = { 1, 2, 3, 4, 5, 6 };\n"
        "    printf(\"%d\\n\", (int)(sizeof(a) / sizeof(a[0])));",
        6,
        "sizeof(a) 是整个数组的字节数，除以单个元素的大小就得到元素个数。\n"
        "  这种写法只对真正的数组有效，对指针无效。"),

    /* ===== 指针与内存 ===== */
    NUM("指针与内存",
        "下面程序的输出是什么？\n"
        "    int a[5] = { 10, 20, 30, 40, 50 };\n"
        "    int *p = a;\n"
        "    printf(\"%d\\n\", *(p + 3) - *p);",
        30,
        "p + 3 指向 a[3]，值为 40；*p 是 a[0]，值为 10；差是 30。\n"
        "  指针加 1 前进的是一个元素的大小，不是 1 字节。"),
    NUM("指针与内存",
        "下面程序的输出是什么？\n"
        "    int arr[] = { 1, 2, 3, 4, 5 };\n"
        "    int *p = arr + 4;\n"
        "    printf(\"%td\\n\", p - arr);",
        4,
        "同类型指针相减得到的是元素个数（不是字节数），结果是 4。\n"
        "  用 %td 打印 ptrdiff_t 类型。"),
    MC("指针与内存",
       "在 64 位平台上，下面程序的输出是什么？\n"
       "    void f(int a[])\n"
       "    {\n"
       "        printf(\"%zu\\n\", sizeof(a));\n"
       "    }",
       "20", "8", "5", "编译错误", 1,
       "数组作函数参数时会退化为指针，sizeof 得到的是指针大小，64 位平台为 8。\n"
       "  所以函数里要用 sizeof 求元素个数，必须额外传一个长度参数。"),
    NUM("指针与内存",
        "下面程序的输出是什么？\n"
        "    int a[3] = { 0 };\n"
        "    a[0] = 1;\n"
        "    int *p = &a[1];\n"
        "    p[-1] = 5;\n"
        "    printf(\"%d\\n\", a[0]);",
        5,
        "p 指向 a[1]，p[-1] 等价于 *(p - 1)，也就是 a[0]，所以 a[0] 被改成 5。\n"
        "  下标运算本质是指针偏移，负数下标只要不越界就是合法的。"),
    MC("指针与内存",
       "下面程序的输出是什么？\n"
       "    char *p = \"hello\";\n"
       "    printf(\"%c\\n\", *(p + 4));",
       "o", "l", "h", "未定义行为", 0,
       "p 指向首字符 'h'，p + 4 指向第 5 个字符 'o'（下标从 0 开始）。"),

    /* ===== 函数与作用域 ===== */
    NUM("函数与作用域",
        "下面程序的输出是什么？\n"
        "    int f(void)\n"
        "    {\n"
        "        static int n = 0;\n"
        "        return ++n;\n"
        "    }\n"
        "    /* 依次调用 f() 三次 */\n"
        "    printf(\"%d\\n\", f());",
        3,
        "static 局部变量只初始化一次，函数返回后值仍然保留：三次调用返回 1、2、3。"),
    NUM("函数与作用域",
        "全局变量 g = 10，下面程序的输出是什么？\n"
        "    int f(void)\n"
        "    {\n"
        "        int g = 3;\n"
        "        return g * 2;\n"
        "    }\n"
        "    printf(\"%d\\n\", f());",
        6,
        "局部变量 g 遮蔽了同名全局变量，函数里用的是 3，3 * 2 = 6。全局变量 g 不受影响。"),
    MC("函数与作用域",
       "关于函数内的 static 局部变量，下面哪种说法是正确的？",
       "每次调用都会重新初始化为 0",
       "只初始化一次，函数返回后仍保留，生命周期到程序结束",
       "作用域扩大到整个文件",
       "存放在栈上，随函数返回被释放", 1,
       "static 局部变量存放在静态存储区，只初始化一次并保留到程序结束；\n"
       "  但它的作用域仍然只在函数内部，这一点和文件作用域的 static 全局变量不同。"),

    /* ===== 预处理 ===== */
    NUM("预处理",
        "下面程序的输出是什么？\n"
        "    #define SQ(x) x * x\n"
        "    printf(\"%d\\n\", SQ(1 + 2));",
        5,
        "宏是纯文本替换：SQ(1 + 2) 展开为 1 + 2 * 1 + 2，按优先级算出 5，不是 9。\n"
        "  正确写法是 #define SQ(x) ((x) * (x))。"),
    NUM("预处理",
        "下面程序的输出是什么？\n"
        "    #define A 2 + 3\n"
        "    printf(\"%d\\n\", A * A);",
        11,
        "替换后是 2 + 3 * 2 + 3 = 11，不是 25。\n"
        "  对象式宏也要把整个表达式括起来：#define A (2 + 3)。"),
    MC("预处理",
       "头文件里防止重复包含的标准写法是？",
       "#define 一次就够，不需要其它处理",
       "#ifndef HEADER_H / #define HEADER_H / ... / #endif",
       "#pragma once 只能用于 C++",
       "用 extern 声明所有内容即可", 1,
       "#ifndef / #define / #endif（或 #pragma once）保证头文件内容只被展开一次，\n"
       "  否则结构体定义重复会报 redifinition 错误。"),
};

static const int nb = (int)(sizeof(bank) / sizeof(bank[0]));

static const char *topics[MAX_TOPICS];
static int ntopics;

static int last_wrong[MAX_Q];
static int last_nwrong;

static int use_color;

static void set_color(const char *code)
{
    if (use_color)
        fputs(code, stdout);
}

static void reset_color(void)
{
    if (use_color)
        fputs("\033[0m", stdout);
}

static int read_line(char *buf, size_t cap)
{
    if (fgets(buf, (int)cap, stdin) == NULL)
        return 0;

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else if (len + 1 == cap) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
    return 1;
}

static void trim(char *s)
{
    char *p = s;
    while (*p != '\0' && isspace((unsigned char)*p))
        p++;
    if (p != s)
        memmove(s, p, strlen(p) + 1);

    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
}

static int topic_index(const char *name)
{
    for (int i = 0; i < ntopics; i++)
        if (strcmp(topics[i], name) == 0)
            return i;
    return -1;
}

static int topic_count(int ti)
{
    int n = 0;
    for (int i = 0; i < nb; i++)
        if (topic_index(bank[i].topic) == ti)
            n++;
    return n;
}

static void build_topics(void)
{
    ntopics = 0;
    for (int i = 0; i < nb; i++) {
        if (topic_index(bank[i].topic) >= 0)
            continue;
        if (ntopics < MAX_TOPICS)
            topics[ntopics++] = bank[i].topic;
    }
}

static void print_correct(const Question *q)
{
    if (q->type == TYPE_MC)
        printf("%c", 'A' + q->mc_answer);
    else
        printf("%ld", q->num_answer);
}

/* 返回 1=正确, 0=错误或放弃, -1=输入格式无效 */
static int judge(const Question *q, const char *ans)
{
    if (*ans == '\0')
        return 0;

    if (q->type == TYPE_MC) {
        if (ans[1] != '\0')
            return -1;
        char c = (char)toupper((unsigned char)ans[0]);
        if (c >= 'A' && c <= 'D')
            return (c - 'A') == q->mc_answer;
        if (c >= '1' && c <= '4')
            return (c - '1') == q->mc_answer;
        return -1;
    }

    char *end = NULL;
    errno = 0;
    long v = strtol(ans, &end, 10);
    if (end == ans || *end != '\0' || errno == ERANGE)
        return -1;
    return v == q->num_answer;
}

typedef struct {
    int asked;
    int correct;
    int wrong[MAX_Q];
    int nwrong;
    int topic_asked[MAX_TOPICS];
    int topic_correct[MAX_TOPICS];
} Session;

static void show_summary(const Session *s)
{
    printf("\n========== 本轮成绩 ==========\n");
    if (s->asked == 0) {
        puts("本轮没有作答。");
        return;
    }

    printf("作答 %d 题，答对 %d 题，正确率 %.0f%%\n",
           s->asked, s->correct, 100.0 * s->correct / s->asked);

    puts("\n各知识点：");
    for (int i = 0; i < ntopics; i++) {
        if (s->topic_asked[i] == 0)
            continue;
        printf("  %s：%d/%d", topics[i], s->topic_correct[i], s->topic_asked[i]);
        if (s->topic_correct[i] == s->topic_asked[i])
            printf("  全部答对");
        printf("\n");
    }

    if (s->nwrong > 0) {
        printf("\n错题回顾（%d 题）：\n", s->nwrong);
        for (int i = 0; i < s->nwrong; i++) {
            const Question *q = &bank[s->wrong[i]];
            printf("\n  · [%s] 正确答案：", q->topic);
            print_correct(q);
            printf("\n    %s\n", q->explain);
        }
    }
}

static void run_session(const int *ids, int n)
{
    Session s;
    memset(&s, 0, sizeof s);
    int quit = 0;
    char line[128];

    for (int k = 0; k < n && !quit; k++) {
        const Question *q = &bank[ids[k]];
        int ti = topic_index(q->topic);

        printf("\n");
        set_color("\033[1m");
        printf("[%d/%d] 知识点：%s\n", k + 1, n, q->topic);
        reset_color();
        printf("----------------------------------------\n");
        printf("%s\n\n", q->stem);

        if (q->type == TYPE_MC) {
            for (int i = 0; i < 4; i++)
                printf("  %c) %s\n", 'A' + i, q->opts[i]);
        } else {
            puts("（填整数结果，直接回车看答案）");
        }

        int verdict = -1;
        for (;;) {
            printf("\n答案> ");
            fflush(stdout);
            if (!read_line(line, sizeof line)) {
                quit = 1;
                break;
            }
            trim(line);
            if (strcasecmp(line, "q") == 0) {
                quit = 1;
                break;
            }
            verdict = judge(q, line);
            if (verdict >= 0)
                break;
            puts("输入无效：选择题答 A-D（或 1-4），填空题答整数。");
        }
        if (quit)
            break;

        s.asked++;
        if (ti >= 0)
            s.topic_asked[ti]++;

        if (verdict == 1) {
            s.correct++;
            if (ti >= 0)
                s.topic_correct[ti]++;
            set_color("\033[32m");
            puts("✓ 正确");
            reset_color();
        } else {
            s.wrong[s.nwrong++] = ids[k];
            set_color("\033[31m");
            printf("✗ 错误，正确答案：");
            print_correct(q);
            printf("\n");
            reset_color();
        }

        set_color("\033[2m");
        printf("  解析：%s\n", q->explain);
        reset_color();
    }

    show_summary(&s);

    if (s.asked > 0) {
        last_nwrong = s.nwrong;
        if (s.nwrong > 0)
            memcpy(last_wrong, s.wrong, sizeof s.wrong[0] * (size_t)s.nwrong);
    }
}

static void shuffle(int *a, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}

static int ask_count(int max)
{
    char line[64];
    printf("抽几题？（1-%d，回车=全部）：", max);
    fflush(stdout);
    if (!read_line(line, sizeof line))
        return 0;
    trim(line);
    if (line[0] == '\0')
        return max;

    char *end = NULL;
    long v = strtol(line, &end, 10);
    if (end == line || *end != '\0' || v < 1 || v > max) {
        puts("数字无效，已取消。");
        return 0;
    }
    return (int)v;
}

static void practice_by_topic(void)
{
    puts("\n知识点：");
    for (int i = 0; i < ntopics; i++)
        printf("  %d) %s（%d 题）\n", i + 1, topics[i], topic_count(i));
    printf("选择知识点（1-%d，0 返回）：", ntopics);
    fflush(stdout);

    char line[64];
    if (!read_line(line, sizeof line))
        return;
    trim(line);

    char *end = NULL;
    long v = strtol(line, &end, 10);
    if (line[0] == '\0' || end == line || *end != '\0')
        return;
    if (v == 0)
        return;
    if (v < 1 || v > ntopics) {
        puts("没有这个知识点。");
        return;
    }

    int ids[MAX_Q];
    int n = 0;
    for (int i = 0; i < nb; i++)
        if (strcmp(bank[i].topic, topics[v - 1]) == 0)
            ids[n++] = i;
    run_session(ids, n);
}

static void show_overview(void)
{
    puts("\n===== 题库概览 =====");
    for (int i = 0; i < ntopics; i++)
        printf("  %d) %s（%d 题）\n", i + 1, topics[i], topic_count(i));
    printf("  合计 %d 题\n", nb);
    puts("  题型：选择题（A-D）与读程序写结果（填整数）");
}

static void show_menu(void)
{
    printf("\n========== C 语法刷题训练 ==========\n");
    printf("题库：%d 题 / %d 个知识点\n", nb, ntopics);
    puts("  1) 顺序刷全部题目");
    puts("  2) 随机抽题");
    puts("  3) 按知识点练习");
    if (last_nwrong > 0)
        printf("  4) 重做上轮错题（%d 题）\n", last_nwrong);
    else
        puts("  4) 重做上轮错题");
    puts("  5) 题库概览");
    puts("  0) 退出");
    printf("选择> ");
    fflush(stdout);
}

int main(void)
{
    srand((unsigned)time(NULL));
    use_color = isatty(STDOUT_FILENO);
    build_topics();

    puts("C 语法刷题训练 —— 答题时输入 q 可随时结束本轮");

    char line[64];
    int running = 1;

    while (running) {
        show_menu();
        if (!read_line(line, sizeof line))
            break;
        trim(line);

        if (line[0] == '\0') {
            continue;
        } else if (strcmp(line, "1") == 0) {
            int ids[MAX_Q];
            for (int i = 0; i < nb; i++)
                ids[i] = i;
            run_session(ids, nb);
        } else if (strcmp(line, "2") == 0) {
            int ids[MAX_Q];
            for (int i = 0; i < nb; i++)
                ids[i] = i;
            shuffle(ids, nb);
            int n = ask_count(nb);
            if (n > 0)
                run_session(ids, n);
        } else if (strcmp(line, "3") == 0) {
            practice_by_topic();
        } else if (strcmp(line, "4") == 0) {
            if (last_nwrong == 0)
                puts("还没有错题，先做一轮吧。");
            else
                run_session(last_wrong, last_nwrong);
        } else if (strcmp(line, "5") == 0) {
            show_overview();
        } else if (strcmp(line, "0") == 0 || strcasecmp(line, "q") == 0) {
            running = 0;
        } else {
            puts("无效选项。");
        }
    }

    puts("再见。");
    return 0;
}