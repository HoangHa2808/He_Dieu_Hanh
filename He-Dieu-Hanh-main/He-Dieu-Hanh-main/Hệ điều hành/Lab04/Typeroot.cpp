#include <iostream.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <alloc.h>

typedef struct {
	char jmp[3];
	char Ver[8];
	unsigned SecSiz;
	char ClustSiz;
	unsigned ResSec;
	char FatCnt;
	unsigned RootSiz;
	unsigned TotSec;
	char Media;
	unsigned FatSiz;
	unsigned TrkSec;
	unsigned HeadCnt;
	unsigned HidSec;

}EntryBpb;

typedef union {
	char Sec[512];
	EntryBpb Entry;
}HopBpb;

typedef struct {
	unsigned S:5;
	unsigned M:6;
	unsigned H:5;
}Time;

typedef union {
	unsigned intTime;
	Time T;
}HopTime;


typedef struct {
	unsigned D:5;
	unsigned M:4;
	unsigned Y:7;
}Date;

typedef union {
	unsigned intDate;
	Date Dat;
}HopDate;

typedef struct{
	char ReadOnly:1;
	char Hidden:1;
	char System:1;
	char Volume:1;
	char SubDir:1;
	char Archive:1;
	char DR:2;
}Attrib;

typedef union {
	char charAtt;
	Attrib Attr;
}HopAttrib;



typedef struct {
		char FileName[8];
		char Ext[3];
		HopAttrib Att;
		char DR[10];
		HopTime Tg;
		HopDate Ng;
		unsigned Clust;
		long FileSize;
}EntryDir;

typedef union {
	char Entry[32];
	EntryDir Entdir;
}HopDir;



typedef struct   Node
	{
		void *Data;
		Node *Next;
	}NodeType;

typedef NodeType *PointerType;



EntryBpb ReadBPB();
void ReadFat();
int InsertLast(PointerType &List,PointerType &Last,void *Item);
int NDTMHienHanh(PointerType ListPath);
void ReadRoot(PointerType &ListRoot);
void Change(long SectorNumber,unsigned int &Side,\
	     unsigned int &Track,unsigned int &Sector);
int ReadDisk(char *Buff,char Odia,long SectorNumber,int SoSector);
int ReadDiskBios(char *Buff,unsigned Side,unsigned Track,unsigned Sector,int SoSector);

void DeleteList(PointerType &ListClust);
unsigned SearchDir(PointerType ListEntry,char *FileName,long &FileSize);
int SoSanh(char *s,char *d);

PointerType GetEntry(unsigned char *BuffDir,unsigned Size);
PointerType GetEntryDir(PointerType ListClust, char flag);
unsigned NextEntry(unsigned Index);
PointerType NDSubDir(unsigned Clust,PointerType ListPath);

PointerType GetCluster(unsigned StartClust);


char *Path;//="A:\\Baitap";
char Odia;
EntryBpb Bpb;
unsigned char *FAT;
long SecStart=0;

//***********************************************************************//
//Chen mot nut vao danh sach
//***********************************************************************//
int InsertLast(PointerType &List,PointerType &Last,void *Item)
{
	PointerType Temp;
	Temp=new NodeType;
	if(!Temp)
	    return 0;

	Temp->Data=Item;
	Temp->Next=NULL;
	if(List==NULL)
		List=Temp;
	else
		Last->Next=Temp;
	Last=Temp;

	return 1;
}


//***********************************************************************//}
//Doc noi dung bang fat vao FAT(bien toan cuc)
//***********************************************************************//}
void ReadFat()
{
	FAT=new char[Bpb.FatSiz*512];
	//FAT=(unsigned char far*)farmalloc(Bpb.FatSiz*512);
	if(!FAT)
		exit(1);

	if(!ReadDisk(FAT,Odia,Bpb.ResSec,Bpb.FatSiz))
		{
			cout<<"\nKhong doc bang Fat";
			exit(1);
		}
}
//***********************************************************************//
//Cho hien thi thu muc da tim duoc
//***********************************************************************//
// int PrintFile(PointerType ListClust, long FileSize)
// {
// 	PointerType Temp=NULL;

// 	unsigned CurrClust;
// 	long CurrSec;
// 	char *Buff;
// 	unsigned Clust;


// 		int i,j=0;
// 		Buff=new char[Bpb.ClustSiz*512];
// 		if(!Buff)
// 		{
// 			cout<<"\nKhong du bo nho ";
// 			exit(1);
// 		}
// 		Temp=ListClust;

// 		while(Temp!=NULL)
// 		{

// 			CurrClust=*(unsigned*)ListClust->Data;
// 			CurrSec=Bpb.ResSec+Bpb.FatSiz*Bpb.FatCnt+(Bpb.RootSiz*32)/512 +(CurrClust-2)*Bpb.ClustSiz;

// 			if(!ReadDisk(Buff,0,CurrSec,Bpb.ClustSiz))
// 			{
// 				cout<<"\nKhong doc dia";
// 				exit(1);
// 			}
// 			i=0;
// 			j=0;
// 			while((i<512) && (j<FileSize))
// 			{
// 				cout<<Buff[i];
// 				i++;
// 				j++;
// 			}


// 			Temp=Temp->Next;
// 		}
// 		delete Buff;
//                 getch();

// }
//***********************************************************************//
//Doc noi dung thu muc goc
//***********************************************************************//
void ReadRoot(PointerType &ListRoot)
{
	HopDir *Dir;

	unsigned size=(Bpb.RootSiz)*32;
	unsigned char *ROOT;
	ROOT=new char[size];
	if(!ROOT)
	{
		cout<<"\nkhong du bo nho";
		exit(1);
	}
	long SecRoot=Bpb.ResSec+Bpb.FatSiz*Bpb.FatCnt;
	 unsigned SoSec=Bpb.RootSiz*32/512;


	if(!ReadDisk(ROOT,Odia,SecRoot,SoSec))
	{
		cout<<"\nKhong doc bang Root";
		exit(1);
	}
	ListRoot=GetEntry(ROOT,size);
	delete ROOT;

}
//***********************************************************************//
//Doi seclog sang sec vat ly
//***********************************************************************//
void Change(long SectorNumber,unsigned int &Side,unsigned int &Track,unsigned int &Sector)
   {
	unsigned int X;

    Sector=(unsigned)(1+(SectorNumber) % Bpb.TrkSec);
    Side=(unsigned)(((SectorNumber)/Bpb.TrkSec)%Bpb.HeadCnt);
    Track=(unsigned)((SectorNumber)/(Bpb.TrkSec*Bpb.HeadCnt));

    X=Track;
    X=X&0xFF00;
    X=X>>2;
    X=X&0x00FF;
    X=X|Sector;
    Track=Track<<8;
    Track=Track|X;  // track(10) sector(6)

   }

//***********************************************************************//}
//Doc dia theo tung seclog
//***********************************************************************//}
// int ReadDisk(char *Buff,char Odia,long SectorNumber,int SoSector)
//   {
// 	unsigned int Side,Track,Sector;
// 	Change(SectorNumber,Side,Track,Sector);
// 	if(ReadDiskBios(Buff,Side,Track,Sector,SoSector))
// 		return 1;
// 	else
// 		return 0;
//   }
//***********************************************************************//
int ReadDiskBios(char *Buff,unsigned Side,unsigned Track,unsigned Sector,int SoSector)
  {
    union REGS u,v;
    struct SREGS s;
    int k,i=0;
    v.x.cflag=1;
    while((i<2)&&(v.x.cflag!=0))
    {
    u.h.ah=0x2;
    u.h.dl=Odia;                   //SO DISK;
    u.h.dh=Side;
	u.h.cl=Sector;
	u.h.ch=Track;                    //MAT DISK;
    u.x.cx=Track;
    u.h.al=SoSector;                      //So sector can doc;
    s.es=FP_SEG(Buff);
    u.x.bx=FP_OFF(Buff);

    int86x(0x13,&u,&v,&s);
    i++;
    }
    // k=v.h.ah;
    return(!v.x.cflag);
  }

//***********************************************************************//}
//Doc noi dung bang tham so dia da duoc sua de doc file
//***********************************************************************//}
void ReadBPB()
{
	HopBpb temp;
	if (Odia == 0 || Odia == 1)
		if(!ReadDiskBios(temp.Sec,0,1,1,1)){
			cout<< "\nKhong doc duoc bang tham so dia!";
			return;
		}
	Bpb = temp.Entry;
}

//***********************************************************************//
//Xoa danh sach lien ket
//***********************************************************************//
void DeleteList(PointerType &ListClust)
{
	PointerType Temp;
	Temp=ListClust->Next;
	while(Temp!=NULL)
	{
		delete ListClust;
		ListClust=Temp;
		Temp=Temp->Next;
	}
	delete ListClust;
}
//***********************************************************************//
//Tim mot thu muc con co trong thu muc khong
//***********************************************************************//
unsigned SearchDir(PointerType ListEntry,char *FileName, long &FileSize)
{
	unsigned Clust=0;
	char Temp[10];
	int i;
	while(ListEntry!=NULL)
	{
		//cout<<FileName<<'\t'<<((EntryDir*)(ListEntry->Data))->FileName;getch();
		// Do Filename trong Entry khong co ky tu null nen chep sang
		// bien temp de so sanh
		i=0;
		while(i<8 && ((EntryDir*)(ListEntry->Data))->FileName[i]!=' ')
		{
			Temp[i]=((EntryDir*)(ListEntry->Data))->FileName[i];
			i++;
		}
		Temp[i]='\0';

		if(strcmp(FileName, Temp)==0)
		{
			Clust=((EntryDir*)(ListEntry->Data))->Clust;
			FileSize=((EntryDir*)(ListEntry->Data))->FileSize;
			return Clust;
		}
		ListEntry=ListEntry->Next;
	}
	return 0;
}
//***********************************************************************//
//So sanh 2 chuoi
//***********************************************************************//
int SoSanh(char *s,char *d)
{

	int i=0;
	while(s[i]!='\0')
	{
		if(s[i]!=d[i])
			return 0;
		i++;
	}
	if(d[i]==' ')
		return 1;
	else
		return 0;
}
//***********************************************************************//
//Dua tung entry thu muc vao danh sach
//***********************************************************************//
PointerType GetEntry(unsigned char *BuffDir,unsigned Size)
{
	HopDir *Dir;

	PointerType ListEntry=NULL,Last=NULL;

	int i=0,j=0;
	while(i<Size)
	{
		if(BuffDir[i]!=0)
		{
			if(BuffDir[i]!=0xE5)
			{
				j=0;
				Dir=new HopDir;
				while(j<32)
				{
					Dir->Entry[j]=BuffDir[i];
					j++;
					i++;
				}
				InsertLast(ListEntry,Last,&Dir->Entdir);
			}
			else
				i=i+32;
		}
		else
			break;
	}
	return ListEntry;

}
//***********************************************************************//
//Tim clust ke tiep cua file trong fat
//***********************************************************************//
unsigned  NextEntry(unsigned Index)
{
	unsigned Addr,X,X1;
	if((Odia==0) || (Odia==1))
	{
		Addr=(Index*3)/2;

		X=FAT[Addr];
		X1=FAT[Addr+1];

		X1=X1<<8;
		X=X1+X;
		if((Index %2) ==0)
			X=X&0x0FFF;
		else
			X=X>>4;

	}
	else
	{
		Addr=(Index*2)-1;
		X=FAT[Addr];
		X1=FAT[Addr+1];

		X1=X1<<8;
		X=X1+X;
	}
	return X;
}
//***********************************************************************//
//Dua tung entry thu muc vao danh sach
//***********************************************************************//
PointerType GetEntryDir(PointerType ListClust, char flag)
{
//flag=0: thu muc goc
//flag=1: thu muc con
	HopDir *Dir;
	unsigned char *BuffDir;
	int i,j;

	PointerType ListEntry=NULL,Last=NULL;
	unsigned CurrClust, CurrSec, Size;
	Size=Bpb.ClustSiz*512;
	BuffDir=new unsigned char[Size];


	while(ListClust!=NULL)
	{
		CurrClust=*(unsigned*)ListClust->Data;
		if(flag==1)
			CurrSec=Bpb.ResSec+Bpb.FatSiz*Bpb.FatCnt+(Bpb.RootSiz*32)/512 +(CurrClust-2)*Bpb.ClustSiz;
		else
			CurrSec=CurrClust;

		if(!ReadDisk(BuffDir,Odia,CurrSec,Bpb.ClustSiz))
		{
		cout<<"\nKhong doc dia";
		exit(1);
		}

		i=0,j=0;
		while(i<Size)
		{
			if(BuffDir[i]!=0)
			{
				if(BuffDir[i]!=0xE5)
				{
					j=0;
					Dir=new HopDir;

					while(j<32)
					{
						Dir->Entry[j]=BuffDir[i];
						j++;
						i++;
					}
					InsertLast(ListEntry,Last,&Dir->Entdir);
				}
				else
					i=i+32;
			}
			else
				break;
		}
		ListClust=ListClust->Next;
	}
	delete BuffDir;
	return ListEntry;

}
//***********************************************************************//
//Lay danh sach cluster thu muc con hay tap tin
//***********************************************************************
PointerType GetCluster(unsigned StartClust)
{
	PointerType ListClust=NULL, Temp=NULL;
	unsigned *Clust, NextClust;


	int i=0;
	NextClust=StartClust;

	while((NextClust>=2)&&(NextClust<0xFEF))
	{
		Clust= new unsigned;
		*Clust=NextClust;

		InsertLast(ListClust, Temp,(unsigned*)Clust);
		NextClust=NextEntry(*Clust);
	}


	return ListClust;
}

//***********************************************************************//
//Lay danh sach cluster thu muc goc
//***********************************************************************
PointerType GetClusterRoot()
{
	PointerType ListClust=NULL,Temp=NULL;
	unsigned SecRoot, *Clust;


	SecRoot=Bpb.ResSec+Bpb.FatSiz*Bpb.FatCnt;
	 unsigned NumSec=Bpb.RootSiz*32/512;

 int i=0;
	while(i<NumSec)
	{
		Clust=new unsigned;
		*Clust=SecRoot;
		InsertLast(ListClust,Temp,(unsigned *)Clust);
		SecRoot++;
		i++;
	}

	return ListClust;
}
/************************************************** ĐỌC FILE BẤT KÌ ***************************************************************/
/* Lấy các thư mục xuất hiện trong đường dẫn, ngoại trừ ổ đĩa.
INPUT:
- path: Chuỗi đường dẫn mà người dùng nhập vào
OUTPUT:
- Null: đường dẫn không hợp lệ
- !Null: danh sách đường dẫn!
*/
PointerType AnalysePath(char *path){
	PointerType list = NULL, last = NULL;
	char *filename;
	drive = Path[0];
	if (drive >= 97 && drive <= 122)
		drive -= 97; //Chữ thường
	else if (drive >= 65 && drive <= 90)
		drive -= 65; //Chữ hoa
	else{
		cout << "\nKhong ton tai o dia! " + drive;
		return NULL;
	}
	if(drive == 2)
		drive = 0x80;
	if (drive == 0||drive == 1 || drive == (char)0x80) //Chỉ nhận ổ A,B,C không nhận ổ đĩa 
	{
		if (Path[1] != ':' && path[2] != '\\'){
			cout << "\nDuong dan nhap da sai! ";
			return NULL;
		}
		if(Path[3] == '\0'){
			cout << "\nDuong dan thiu thu muc!";
			return NULL;
		}
		int i=3;
		while (Path[i] != "\0") //Đọc hết đường dẫn
		{
			int j = 0;
			filename = new char[12];
			if(!filename){
				cout << "\nLoi cap phat vung nho cho ten file";
				return NULL;
			}
			while (Path[i] != '\\' && Path[i] != '\0')//Đọc hết tên của thư mục
			{
				filename[j] = Path[i];
				++i;
				++j;
			}
			filename = strupr(filename); //Đổi tên file sang chữ hoa
			filename[j] = '\0';
			j=0;
			while (filename[j] != '.' && filename[j] != '\0')
				++j;
			if (filename[j] == '.')//Nếu nó là tập tin
			{
				filename[j] = ' ';
				++j;
				int k=3;
				while (filename[j] != '\0')
				{
					filename[11 - k] = filename[j];
					++j;
				}
				filename[11] = '\0';
				k = 7;
				while (filename[k] != ' ')
				{
					filename[k] = ' ';
					k--;
				}
			}
			if (Path[i] != '\0')
				i++;
			InsertLast(list,last,filename);
		}
		return list;
		
	}
	else{
		cout<< "\nDuong dan thieu o dia!";
		return NULL;
	}
}

/*Kiểm tra một thư mục con có xuất hiện trong thư mục được chỉ định
INPUT: listEntry: Danh sách entry của một thư mục (gốc hoặc con)
-filename: Tên của thư mục con, - dir: Thư mục tìm được
OUTPUT:
- true: Tìm thấy
- false: Không tìm thấy
*/
int SearchDir2(PointerType listEntry, char *filename, EntryDir &dir){
	PointerType p = listEntry;
	while (p!= NULL)
	{
		if(Compare(filename,((EntryDir *)p->Data)->FileName)){
			dir = *(EntryDir *)p->Data;
			return 1;
		}
		p = p->Next;
	}
	return 0;

}

//Đọc đĩa theo từng sector Logic
int ReadDisk2(char *buff, long begin, int number){
	unsigned side, track, sector;
	Change(begin,side,track,sector);
	if(ReadDiskBios(buff,side,track,sector,number))
		return 1;
	else return 0;
}

//Đọc nội dung bảng FAT vào biến toàn cục
int ReadFat2(){
	FAT = new char[Bpb.FatSiz * Bpb.SecSiz];
	if(!FAT){
		cout<<"\nLoi cap phat bo nho";
		return 0;
	}
	if (!ReadDisk2(FAT,Bpb.ResSec,Bpb.FatSiz)){
		cout<<"\nKhong doc duoc bang tham so dia!";
		return 0;
	}
	return 1;
}

//Lấy danh sách cluster của thư mục gốc
PointerType GetClusterRoot2(){
	PointerType listCluster = NULL, last =NULL;
	unsigned rootSec, *cluster, number;
	rootSec = Bpb.ResSec + Bpb.FatSiz * (int)Bpb.FatCnt;
	number = Bpb.RootSiz * 32/512;
	for(int i =0;i<number;i++,rootSec++){
		cluster = new unsigned;
		*cluster = rootSec;
		InsertLast(listCluster,last,(unsigned *)cluster);
	}
	return listCluster;
}

//Lấy danh sách cluster đưa vào cluster bắt đầu
PointerType GetCluster2(unsigned begin){
	PointerType listCluster = NULL, last =NULL;
	unsigned *cluster, next = begin;
	for (; next >= 2 && next < 0xFEF; next = NextEntry(*cluster))
	{
		cluster = new unsigned;
		*cluster = next;
		InsertLast(listCluster, last, (unsigned *)cluster);
	}
	return listCluster;
}

//Đọc từng entry bên trong thư mục và cho vào danh sách
PointerType GetEntryDir2(PointerType listCluster, char flag)
{
	PointerType p = listCluster, listEntry = NULL, last = NULL;
	unsigned size = Bpb.ClustSiz * 512, currentCluster, currentSector;
	unsigned char *buff = new unsigned char[size];
	UnionDir *dir;
	while (p != NULL)
	{
		currentCluster = *(unsigned *)p->Data;
		if (flag)
			currentSector = Bpb.ResSec + Bpb.FatSiz * Bpb.FatCnt + (Bpb.RootSiz * 32) / 512 + (currentCluster - 2) * Bpb.ClustSiz;
		else
			currentSector = currentCluster;
		if (!ReadDisk(buff, currentSector, Bpb.ClustSiz))
		{
			cout << "\nKhong doc duoc dia voi danh sach cluster nay";
			return NULL;
		}
		for (int i = 0; i < size;)
		{
			if (buff[i] != (char)0x00)	//Entry khac trong
				if (buff[i] != (char)0xE5)	//Entry khong bi xoa
				{
					int j = 0;
					dir = new UnionDir;
					for (; j < 32; j++, i++)
						dir->Entry[j] = buff[i];
					InsertLast(listEntry, last, &dir->Entdir);
				}
				else i += 32;
			else break;
		}
		p = p->Next;
	}
	delete buff;
	return listEntry;
}

//Lấy kích thước trên đĩa dựa vào kích thước thật
long GetSizeOnDisk(long realSize)
{
	long size = (long)Bpb.SecSiz * Bpb.ClustSiz; //Bytes/cluster
	if ((realSize % size) != 0)
		return ((realSize / size) + 1) * size;
	else
		return realSize;
}

//Hiển thị nội dung của thư mục lên màn hình
void PrintDir(PointerType listEntry)
{
	PointerType p = listEntry;
	EntryDir dir;
	cout << '\n'
		<< setiosflags(ios::left)
		<< setw(8) << "Name"
		<< setw(8) << "Ext"
		<< setw(16) << "Size (bytes)"
		<< setw(16) << "On disk (bytes)"
		<< setw(8) << "Time"
		<< setw(12) << "Date";
	while (p != NULL)
	{
		dir = *(EntryDir *)p->Data;
		if (dir.Att.Attr.Volume == 0)
		{
			int i;
			cout << "\n";
			for (i = 0; i < 8; i++)
				cout << dir.FileName[i];
			for (i = 0; i < 3; i++)
				cout << dir.Ext[i];
			cout << '\t' << dir.FileSize
				<< "\t\t" << GetSizeOnDisk(dir.FileSize)
				<< "\t\t" << ((dir.Tg).T).H << ':' << ((dir.Tg).T).M
				<< '\t' << ((dir.Ng).Dat).D << '/' << ((dir.Ng).Dat).M << "/" << ((dir.Ng).Dat).Y + 1980;
		}
		p = p->Next;
	}
}
//***********************************************************************//
//Ham main
//***********************************************************************//
void ChayChuongTrinh()
{
	path = new char[256];
	cout << "\nNhap duong dan: ";
	scanf("%s", path);
	PointerType listPath = AnalysePath(path);
	if (listPath == NULL)
		return;
	ReadBPB();
	if (!ReadFat2())
		return;
	//Tai thu muc goc
	PointerType listCluster = GetClusterRoot2();
	PointerType listEntry = GetEntryDir2(listCluster, 0);
	//Tai thu muc con
	while (listPath)
	{
		EntryDir dir;
		if (SearchDir2(listEntry, (char *)listPath->Data, dir))
		{
			listCluster = GetCluster2(dir.Clust);
			listEntry = GetEntryDir2(listCluster, 1);
		}
		else
		{
			cout << "\nKhong tim thay thu muc " << (char *)listPath->Data;
			return;
		}
		listPath = listPath->Next;
	}
	PrintDir(listEntry);
}

int main(int argc,char *argv[])
{
	clrscr();
	ChayChuongTrinh();
	getch(); 
	return 1;
}


