// these edits need to be made to
// variant.cpp
/*

// at the end of the file (replace other serial instantiations) 
// (use of ser3 not compatible with Wire)
// (seems possible to use sercom5, to release wire)
// we have SER0, SER1, SER2, SER5 available... 

// this confirmed OK, it's default, 
Uart Serial1( &sercom0, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX ) ;
void SERCOM0_Handler()
{
  Serial1.IrqHandler();
}

// this one's fine, 
Uart Serial2( &sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2 );
void SERCOM1_Handler(){
  Serial2.IrqHandler();
}

// inventing another...
Uart Serial3( &sercom4, 22, 23, SERCOM_RX_PAD_0, UART_TX_PAD_2 );
void SERCOM4_Handler(){
  Serial3.IrqHandler();
}

// and... sercom3 is Wire, so sercom2 would be the look, 
Uart Serial4( &sercom3, 12, 6, SERCOM_RX_PAD_3, UART_TX_PAD_2 );
void SERCOM3_Handler(){
  Serial4.IrqHandler();
}

*/
// variant.h
/*
extern Uart Serial1;
extern Uart Serial2;
extern Uart Serial3;
extern Uart Serial4; 
*/