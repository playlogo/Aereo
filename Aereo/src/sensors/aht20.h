#ifndef AHT20_H
#define AHT20_H

bool setupAHT20AndENS160();

void loopAHT20(void *parameter);
void loopENS160(void *parameter);

#endif