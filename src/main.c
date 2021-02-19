volatile unsigned int *led = (unsigned int*) 0x80000000; // memory address of led peripheral

unsigned int change_led(unsigned int val){
 	*led = val;
	return 0;
}

int main(){
  unsigned int i = 0;

  *led = 0x55;

  while(1) {
    change_led(i);
    i++;
  }
}
