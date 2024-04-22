#include "stdio.h"


int InBlock[81], InRow[81], InCol[81];// 从Square到对应block, row, col的映射。

const int BLANK = 0;
const int ONES = 0x3fe; 	// Binary 1111111110

int Entry[81];	// Records entries 1-9 in the grid, as the corresponding bit set to 1
int Block[9], Row[9], Col[9];	// Each int is a 9-bit array ；；； 其实这是每格、每行、每列现在已有的值。因为数独规则就是每行每列每格都要1-9不重复填满。

int SeqPtr = 0;
int Sequence[81]; // 一个1-81的排列。请问这有什么用。？？

int Count = 0;
int LevelCount[81];


void SwapSeqEntries(int S1, int S2)// Sequence的S1, S2项交换
{
     int temp = Sequence[S2];
     Sequence[S2] = Sequence[S1];
     Sequence[S1] = temp;
}


void InitEntry(int i, int j, int val) // 初始化。
{
	 int Square = 9 * i + j;
	 int valbit = 1 << val;
     int SeqPtr2;

     // add suitable checks for data consistency
     
	 Entry[Square] = valbit; // 设置Entry对应valbit
	 Block[InBlock[Square]] &= ~valbit; // 这是给对应Block去掉已有的值。下面的也是，去掉Row, Col空白填写的可能性。
	 Col[InCol[Square]] &= ~valbit; // Simpler Col[j] &= ~valbit;
	 Row[InRow[Square]] &= ~valbit; // Simpler Row[i] &= ~valbit;


     // 现在先来Probe一次SeqPtr

     SeqPtr2 = SeqPtr; // 从SeqPtr出发，找Square。然后换回来。然后SeqPtr自增为1。
     // 我深度怀疑这个东西的效果。SeqPtr指的不就是Sqare么，还用找么？还用交换么？
     while (SeqPtr2 < 81 && Sequence[SeqPtr2] != Square)
           SeqPtr2++ ;

     SwapSeqEntries(SeqPtr, SeqPtr2);
     SeqPtr++;
}


void PrintArray() // Entry的元素是valbit。Sudoku2运用位运算。现在转回来打印出来。
{
     int i, j, valbit, val, Square;// Entry是棋盘。Square是游标。
     char ch;
     
     Square = 0;

     for (i = 0; i < 9; i++) {
         if (i % 3 == 0) putc('\n', stdout);
         for (j = 0; j < 9; j++) {
             if (j % 3 == 0) putc(' ', stdout); // 逢三的地方断开，更好展示。
             valbit = Entry[Square++];
             if (valbit == 0) ch = '-';
             else {
                 for (val = 1; val <= 9; val++) // 其实这个可以直接打表。
                     if (valbit == (1 << val)) {
                        ch = '0' + val;
                        break;
                     }
             }    
             putc(ch,stdout);
         }
         putc ('\n', stdout);
     }
}


void ConsoleInput()// 一行一行地输入，每个元素都调用 InitEntry 。
{
     int i, j;
     char InputString[80];

     for (i = 0; i < 9; i++) {
         printf("Row[%d] : ", i + 1); // 读一行为字符串
         scanf("%s", InputString);

         for (j = 0; j < 9; j++) {
             char ch = InputString[j];
             if (ch >= '1' && ch <= '9') 
                InitEntry(i, j, ch - '0'); // 依次处理。
         }
     }

     PrintArray(); // 最后打印Entry
}


void PrintStats()
{
    int i, j, S;

    printf("\nLevel Counts:\n\n");

    S = 0;
    while (LevelCount[S] == 0) S++; // 寻找S，使得LevelCount不是0

    i = 0;

    while (S < 81) {
          int Seq = Sequence[S]; // 又去Sequence找东西。
          printf("(%d, %d):%4d ", Seq / 9 + 1, Seq % 9 + 1, LevelCount[S]); // 按格式打印Seq/9 Seq%9, LevelCount
          if (i++ > 4){
             printf("\n"); // 控制一行6个
             i = 0;
          }
          S++;
    }

    printf("\n\nCount = %d\n", Count); // 打印Count
}


void Succeed()
{
     PrintArray();
     PrintStats();
}


int NextSeq(int S) // 从S开始往Sequence的后面找。看看哪个格子的限制最多。
{
    int S2, Square, Possibles, BitCount; // S2是使得最小BitCount Possibles的Sequence下标
    int T, MinBitCount = 100;

    for (T = S; T < 81; T++) { // 81次，Sequence里找Possibles的BitCount最小的那个。
        Square = Sequence[T];
        Possibles = Block[InBlock[Square]] & Row[InRow[Square]] & Col[InCol[Square]]; // 这个Entry到底能填啥？我们用位运算来模拟小集合的运算，真不错。
        BitCount = 0; // 现在给比特1计数。就是把Possibles的集合表示转换为实数计数。
        while (Possibles) { // 不停抹掉低位1。这是在给1计数。
           Possibles &= ~(Possibles & -Possibles); // 求最低位的1。然后取反。意味着抹掉低位1。
           BitCount++;
        }

        if (BitCount < MinBitCount) { // 统计最小的BitCount，并登记此时的T。
           MinBitCount = BitCount;
           S2 = T;
        }
    }

    return S2;
}


void Place(int S) // 这是个剪枝的搜索。其实。！！
{
    LevelCount[S]++;
    Count++;

    if (S >= 81) { // S到达81了。搜索到底了。
       Succeed();
       return;
    }

    int S2 = NextSeq(S); // 获取剩余待定序列中可能性最小的那个Entry。优先确定它有利于尽快算出结果，缩小状态空间（十分重要）
    SwapSeqEntries(S, S2); // 这是在调整确定Entry值的先后顺序。至于怎么换过来后面又换回去了。其实是为了恢复计算的状态。

    int Square = Sequence[S];

    int 	BlockIndex = InBlock[Square],
			RowIndex = InRow[Square],
			ColIndex = InCol[Square];

    int 	Possibles = Block[BlockIndex] & Row[RowIndex] & Col[ColIndex];
    while (Possibles) { // 做Possibles次Place，就是树状向下搜索中。
          int valbit = Possibles & (-Possibles); // Lowest 1 bit in Possibles
          Possibles &= ~valbit;
          Entry[Square] = valbit;
          Block[BlockIndex] &= ~valbit;
          Row[RowIndex] &= ~valbit;
          Col[ColIndex] &= ~valbit;
				
          Place(S + 1);

          Entry[Square] = BLANK; // Could be moved out of the loop
          Block[BlockIndex] |= valbit;
          Row[RowIndex] |= valbit;
          Col[ColIndex] |= valbit;
	}

    SwapSeqEntries(S, S2); // 搜完这层之后恢复计算顺序（有必要回复么）
}


int main(int argc, char* argv[])
{
	int i, j, Square;

	for (i = 0; i < 9; i++)
		for (j = 0; j < 9; j++) { // 这是用Square指标来反查Row Col和Block的，免得/9 %9麻烦。这里倒是直接打表了。6666。
        // 理论上来讲一个能随机访问的数组属于一个映射，也可以当一个函数用。这是重要的启发。
        // Haskell的列表是广泛的无穷列表，只能从头读。
			Square = 9 * i + j;
			InRow[Square] = i;
			InCol[Square] = j;
			InBlock[Square] = (i / 3) * 3 + ( j / 3);
		}


	for (Square = 0; Square < 81; Square++) {
        Sequence[Square] = Square; // Seq初始化为各自。这是什么精妙算法？有点印象。
		Entry[Square] = BLANK; // 其实不用初始化他也默认是0的。
        LevelCount[Square] = 0;
    }

	for (i = 0; i < 9; i++) 
		Block[i] = Row[i] = Col[i] = ONES; // 666。映射到每Block, Row, Col，然后用的是位来表示。好compact的代码。

    ConsoleInput();
    Place(SeqPtr);
    printf("\n\nTotal Count = %d\n", Count);

	return 0;
}




