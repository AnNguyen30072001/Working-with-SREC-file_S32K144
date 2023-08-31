# Working with SREC file on S32K144

This project is implemented on S32K144-Q100 Evaluation Board.
Build a program to communicate with PC via UART. Use UART0 (or UART1) for receiving SREC file sent from PC. The file received goes through checking process to make sure it is in correct SREC format and contains no frame error. If no error, send back the infomation of each SREC record (address and data) back to PC using the corresponding UART instance.