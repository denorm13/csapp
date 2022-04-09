#include<stdio.h>

struct rec{
	char a[3];
	int i;
	struct rec *next;
};

void set_val(struct rec *r, char val)
{
	while(r)
	{
		int i=r->i;
		r->a[i]=val;
		r=r->next;
	}
}

int main(int argc, char *argv)
{
	if(argc <3)
	{
		printf("error");
		exit(1);
	}
	struct rec r;
	r.i=argv[1];
	r.next=NULL;
	set_val(&r, argv[2]);

	return 0;
}
