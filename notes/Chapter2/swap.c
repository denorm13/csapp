void inplace_swap(int *x, int *y)
{
    /* a=a^b^b, b=a^b^a*/
    *x=*x^*y;
    *y=*x^*y;
    *x=*x^*y;
}

void reverse_array(int a[], int cnt)
{
    for(int first=0, last=cnt-1; first<last; first++, last--)
    {
        inplace_swap(&a[first], &a[last]);
    }
}