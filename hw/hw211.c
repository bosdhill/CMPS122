#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void bar(char *a) {
	for (int i=0; i < strlen(a); i++){
		if (a[i] == ';') {
			a[i] = '\\';
			a[i+1] = ';';
		}
	}
	printf("system(%s)\n",a);
}

void baz(char* a[] , size_t n) {
	for (int i = 0; i < n; i++) {
		free (a[i]);
	}
}

int main() {
	char *a[10];
	for (int i = 0; i < 10; i++) {
		a[i] = calloc(1, sizeof("hello, world!"));
		strcpy(a[i], "hello, world!");
	}
	printf("a = 0x%x\n", a);
	for (int i = 0; i < 10; i++) {
		printf("a[%d] = %s &a[i] = 0x%x\n", i, a[i], a[i]);
	}
	baz(a, 10);
	printf("a = 0x%x\n", a);
	for (int i = 0; i < 10; i++) {
		printf("a[%d] = %s &a[i] = 0x%x\n", i, a[i], a[i]);
	}
}
