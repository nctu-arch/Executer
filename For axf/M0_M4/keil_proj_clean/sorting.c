   int c, first, last, n, search, array[100];
   int found=0;
 
   printf("Enter number of elements\n");
   scanf("%d",&n);
 
   printf("Enter %d integers\n", n);
 
   for (c = 0; c < n; c++)
      scanf("%d",&array[c]);
 
   printf("Enter value to find\n");
   scanf("%d", &search);
 
   first = 0;
   last = n - 1;
 
   while (first <= last) {
      if(array[first]==search){
		printf("%d found at location %d.\n",search,first);
		found = 1;
		}
	  

		first++;
   }
   if (found==0)
      printf("Not found! %d is not present in the list.\n", search);