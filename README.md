Casio-CAN {#mainpage}
============

Programmable digital clock by receiving messages through the CAN bus.
-------------

The operation of the program is divided into the following .c files, where the **main** one uses the *specific* functions of the rest:

1. **main**
2. **app_serial**
3. **app_clock**
4. **app_bsp**
5. **app_ints**

The header files are the following:

1. **app_bsp**
2. **app_serial**
3. **app_clock**

> The rest of the .c and *header* files are part of the **HAL Library**.

The main code is the following:

```C
int main( void )
{
    HAL_Init();
    Serial_Init();
    Clock_Init();
    LED_Init();
    initialise_monitor_handles();
    Dog_Init();

    while( 1 )
    {
        Serial_Task();
        Clock_Task();
        Heart_Beat();
        Pet_The_Dog();
    }
}
```
The **Serial_Task()** function is ruled by the following state machine:

![Serial State Machine](https://bitbucket.org/rodrigolosal/modularmx/raw/21139d10be254557d809af59d61db459ab396af9/Diagrams/Part-1.png)

The **Clock_Task()** function is ruled by the following state machine:

![Clock State Machine](https://bitbucket.org/rodrigolosal/modularmx/raw/21139d10be254557d809af59d61db459ab396af9/Diagrams/Part-2.jpg)