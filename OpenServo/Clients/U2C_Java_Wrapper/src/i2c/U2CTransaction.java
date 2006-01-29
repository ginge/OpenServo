package i2c;

/**
 * <P>This class holds the information for a read or write transaction.
 * Futhermore it helps handling unsigned bytes.</p>
 *
 * <hr>
 * <p>Copyright (c) 2006, Stefan Engelke (contact@stefanengelke.de)<br>
 * All rights reserved.</p>
 *
 * <p>Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:</p>
 *
 * <p>* Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.</p>
 *
 * <p>* Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.</p>
 *
 * <p>* Neither the name of the copyright holders nor the names of
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.</p>
 *
 *  <p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.</p>
 *
 * @author Stefan Engelke
 * @version 1.0
 */


public class U2CTransaction {
  public int nSlaveDeviceAddress;       /*!< I2C Slave device address in 7 bit format */
  public int nMemoryAddressLength;      /*!< Slave Device Internal address size */
  public int nMemoryAddress;            /*!< Slave Device Internal address (Memory address) */
  public int nBufferLength;             /*!< Transaction buffer length. Can be from 1 up to 256 */
  public byte[] buffer = new byte[256]; /*!< Transaction Buffer */

  /**
   * returns a BIT representation of a byte in the buffer.
   * @param index int
   * @return String
   */
  public String getBits(int index) {
    byte[] bit={0x01,0x02,0x04,0x08,0x10,0x20,0x40,(byte)0x80};
    String result="";
    for (int i=0;i<8;i++) {
      result+=((bit[7-i]&buffer[index])!=0)+"|";
    }
    return result;
  }

  /**
   * returns a HEX representation of a byte in the buffer.
   * @param index int
   * @return String
   */
  public String getHex(int index) {
    String[] hexTab={"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};
    byte b=buffer[index];
    return "0x"+hexTab[(b>>4)&0x0F]+hexTab[b&0x0F];
  }

  /**
   * returns the unsigned value of a byte in the buffer.
   * @param index int
   * @return int
   */
  public int getUByte(int index) {
    return buffer[index]&0xFF;
  }

  /**
   * sets the unsigned value of a byte in the buffer.
   * @param index int
   * @param value int
   */
  public void setUByte(int index, int value) {
    buffer[index]=(byte)(value&0xFF);
  }

  /**
   * returns a word value of two bytes in the buffer.
   * @param index int Index of the first byte. This byte is used as HI- and
   * the following byte as LO-component.
   * @return int 16-bit unsigned word value
   */
  public int getUWord(int index) {
    return getUByte(index)*0xFF+getUByte(index+1);
  }

  /**
   * sets a word value to two bytes in the buffer.
   * @param index int Index of the first byte. This byte is used as HI- and
   * the following byte as LO-component
   * @param value int 16-bit unsigned word value
   */
  public void setUWord(int index, int value) {
    setUByte(index, (value >> 8) & 0xFF);
    setUByte(index+1, value & 0xFF);
  }
}
