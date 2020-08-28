#include <SPI.h>
#include "at86rf2xx.h"

int received = 0;



void setup() {
  while (!SerialUSB) ;
  SerialUSB.begin(115200);
  SerialUSB.println("Starting..");
  at86rf2xx.init(PIN_SPI_SS, PIN_SPI_IRQ , PIN_SPI_SLEEP_TR, PIN_SPI_RESET);
  at86rf2xx.set_chan(20); // set channel to 20
}

void loop() {
  if (at86rf2xx.events)
    at86rf2xx_eventHandler();
  return;
}

void at86rf2xx_eventHandler() {
  /* One less event to handle! */
  at86rf2xx.events--;

  /* If transceiver is sleeping register access is impossible and frames are
   * lost anyway, so return immediately.
   */
  byte state = at86rf2xx.get_status();
  if(state == AT86RF2XX_STATE_SLEEP)
    return;

  /* read (consume) device status */
  byte irq_mask = at86rf2xx.reg_read(AT86RF2XX_REG__IRQ_STATUS);

  /*  Incoming radio frame! */
  if (irq_mask & AT86RF2XX_IRQ_STATUS_MASK__RX_START)
    SerialUSB.println("[at86rf2xx] EVT - RX_START");

  /*  Done receiving radio frame; call our receive_data function.
   */
  if (irq_mask & AT86RF2XX_IRQ_STATUS_MASK__TRX_END)
  {
    if(state == AT86RF2XX_STATE_RX_AACK_ON || state == AT86RF2XX_STATE_BUSY_RX_AACK) {
      SerialUSB.println("[at86rf2xx] EVT - RX_END");
      at86rf2xx_receive_data();
    }
  }
}

void at86rf2xx_receive_data() {
  /*  print the length of the frame
   *  (including the header)
   */
  size_t pkt_len = at86rf2xx.rx_len();
  SerialUSB.print("Frame length: ");
  SerialUSB.print(pkt_len);
  SerialUSB.println(" bytes");

  /*  Print the frame, byte for byte  */
  SerialUSB.println("Frame dump (ASCII):");
  uint8_t data[pkt_len];
  at86rf2xx.rx_read(data, pkt_len, 0);
  for (int d=0; d<pkt_len; d++)
    SerialUSB.print((char)data[d]);
  SerialUSB.println();

  /* How many frames is this so far?  */
  SerialUSB.print("[[Total frames received: ");
  SerialUSB.print(++received);
  SerialUSB.println("]]\n");
}
