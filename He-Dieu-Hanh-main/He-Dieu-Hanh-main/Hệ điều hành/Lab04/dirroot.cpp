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
void ReadRoot(PointerType &ListRoot);
void Change(long SectorNumber,unsigned int &Side,\
	     unsigned int &Track,unsigned int &Sector);
int ReadDisk(char *Buff,char Odia,long SectorNumber,int SoSector);
int ReadDiskBios(char *Buff,unsigned Side,unsigned Track,unsigned Sector,int SoSector);
void PrintTo(PointerType &ListEntry);
PointerType GetEntry(unsigned char *BuffDir,unsigned Size);

char Odia;
EntryBpb Bpb;

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
//Doc noi dung bang tham so dia
//***********************************************************************//}
EntryBpb ReadBPB()
{
	HopBpb bpb;
	EntryBpb temp;
	int i;
    char Buff[512];
      if(!ReadDiskBios(Buff,0,1,1,1))
	{
		cout<<"\nKhong doc duoc bang tham so dia";
		exit(1);
	}


	i=0;
	while(i<512)
	{
		bpb.Sec[i]=Buff[i];
		i++;
	}
	temp=bpb.Entry;
		return temp;
}
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
void Change(long SectorNumber,unsigned int &Side,\
	     unsigned int &Track,unsigned int &Sector)
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
int ReadDisk(char *Buff,char Odia,long SectorNumber,int SoSector)
  {
	unsigned int Side,Track,Sector;
	Change(SectorNumber,Side,Track,Sector);
	if(ReadDiskBios(Buff,Side,Track,Sector,SoSector))
		return 1;
	else
		return 0;
  }
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
    u.h.dl=0;                   //SO DISK;
    u.h.dh=Side;                      //MAT DISK;
    u.x.cx=Track;
    u.h.al=SoSector;                      //So sector can doc;
    s.es=FP_SEG(Buff);
    u.x.bx=FP_OFF(Buff);

    int86x(0x13,&u,&v,&s);
    i++;
    }
    k=v.h.ah;
    return(!v.x.cflag);
  }


//***********************************************************************//
//Hien thi noi dung thu muc len man hinh
//***********************************************************************//
void PrintTo(PointerType &ListEntry)
{
	PointerType Temp;
	Temp=ListEntry;
	int i;
	EntryDir Dir;

	while(Temp!=NULL)
	{
		Dir=*(EntryDir*)(Temp->Data);
		if(Dir.Att.Attr.Volume==0)
		{
			cout<<'\n';
			for(i=0;i<8;i++)
				cout<<Dir.FileName[i];
			cout<<'\t';
			for(i=0;i<3;i++)
				cout<<Dir.Ext[i];
			cout<<'\t';
			cout<<Dir.FileSize;

			cout<<'\t';
			cout<<((Dir.Tg).T).H<<':'<<((Dir.Tg).T).M; //<<':'<<((Dir.Tg).T).S;

			cout<<"\t";
			cout<<((Dir.Ng).Dat).D<<'-'<<((Dir.Ng).Dat).M<<'-'<<1980+((Dir.Ng).Dat).Y;
		 }
		Temp=Temp->Next;
	}
	//DeleteList(ListEntry);
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
//Ham main
//***********************************************************************//

int main(int argc,char *argv[])
{

	PointerType ListEntry;

	clrscr();
	Bpb=ReadBPB();
	ReadRoot(ListEntry);
	PrintTo(ListEntry);


	return 1;
}