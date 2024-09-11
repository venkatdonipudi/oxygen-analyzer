#include <stdint.h>
#include <stdio.h>
#define I2C_TIMEOUT 10000  // Timeout in milliseconds
#define GPIOA_BASE         0x50000000

// GPIOA registers (offsets)
#define GPIOA_MODER        (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRL         (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

// Constants
#define ADS1115_ADDRESS 0x48  // ADS1115 I2C Address
#define CONFIG_REG 0x01
#define CONVERSION_REG 0x00

// Pointer Mask
#define ADS1X15_REG_POINTER_MASK (0x03)      // Mask for register pointers

// Macros for peripheral base addresses
#define RCC_BASE           0x40021000
#define GPIOB_BASE         0x50000400
#define USART2_BASE        0x40004400
#define I2C1_BASE          0x40005400

// RCC registers (offsets)
#define RCC_IOPENR         (*(volatile uint32_t *)(RCC_BASE + 0x34))
#define RCC_APBENR1        (*(volatile uint32_t *)(RCC_BASE + 0x3C))

// GPIO registers (offsets for GPIOB)
#define GPIOB_MODER        (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_AFRH         (*(volatile uint32_t *)(GPIOB_BASE + 0x24)) 
#define GPIOB_PUPDR        (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))

// USART2 registers (offsets)
#define USART2_CR1         (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_BRR         (*(volatile uint32_t *)(USART2_BASE + 0x0C))
#define USART2_ISR         (*(volatile uint32_t *)(USART2_BASE + 0x1C))
#define USART2_TDR         (*(volatile uint32_t *)(USART2_BASE + 0x28))

// I2C1 registers (offsets)
#define I2C1_CR1           (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2           (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_TIMINGR       (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_ISR           (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_TXDR          (*(volatile uint32_t *)(I2C1_BASE + 0x28))
#define I2C1_RXDR          (*(volatile uint32_t *)(I2C1_BASE + 0x24))

// Global Variables
int16_t adc_OOM202;
float mV_OOM202;
float current_O2Percent;
const float mV_baselineO2 = 1000.0; // Set your baseline O2 voltage in mV
const float ambientAir_O2Percent = 20.9; // Typical ambient air O2 percentage

// Function prototypes
void UART2_Init(void);
void I2C1_Init(void);
void delay(uint32_t ms);
void UART_SendString(char *str);
void configureADS1115(void);
int16_t readADS1115(void);
void readOOM202(void);

int main(void)
{
    UART2_Init();          // Initialize UART
    I2C1_Init();           // Initialize I2C

    UART_SendString("Oxygen Analyzer\r\n");

    configureADS1115();  // Configure the ADS1115

    while (1)
    {
        readOOM202();  // Read O2 value and send over UART

        // Delay for 2 seconds
        delay(2000);
    }
}

// UART2 Initialization using registers
void UART2_Init(void)
{
    // Enable GPIOA and USART2 clock
    RCC_IOPENR |= (1 << 0);   // Enable GPIOA clock
    RCC_APBENR1 |= (1 << 17); // Enable USART2 clock

    // Configure PA2 and PA3 as alternate function (AF1) for USART2
    GPIOA_MODER &= ~(3 << (2 * 2));  // Clear mode for PA2
    GPIOA_MODER |= (2 << (2 * 2));   // Set alternate function mode for PA2
    GPIOA_MODER &= ~(3 << (3 * 2));  // Clear mode for PA3
    GPIOA_MODER |= (2 << (3 * 2));   // Set alternate function mode for PA3

    GPIOA_AFRL |= (1 << 8) | (1 << 12);  // Set AF1 (USART2) for PA2 and PA3

    // Configure USART2
    USART2_BRR = 16000000 / 9600;     // Set baud rate (assuming 16 MHz clock)
    USART2_CR1 = (1 << 3) | (1 << 2);   // Enable TX, RX
    USART2_CR1 |= (1 << 0);             // Enable USART2
}

// I2C1 Initialization using PB8 and PB9
void I2C1_Init(void)
{
    // Enable GPIOB and I2C1 clock
    RCC_IOPENR |= (1 << 1);   // Enable GPIOB clock
    RCC_APBENR1 |= (1 << 21); // Enable I2C1 clock

    // Configure PB8 (SCL) and PB9 (SDA) as alternate function (AF6)
    GPIOB_MODER &= ~(3 << (8 * 2));  // Clear mode for PB8
    GPIOB_MODER |= (2 << (8 * 2));   // Set alternate function mode for PB8
    GPIOB_MODER &= ~(3 << (9 * 2));  // Clear mode for PB9
    GPIOB_MODER |= (2 << (9 * 2));   // Set alternate function mode for PB9

   GPIOB_AFRH &= ~(0xF << (0 * 4));   // Clear AFRH8 bits (position 0:3 for PB8)
    GPIOB_AFRH |=  (0x6 << (0 * 4));   // Set AF6 (0110) for PB8

    GPIOB_AFRH &= ~(0xF << (1 * 4));   // Clear AFRH9 bits (position 4:7 for PB9)
    GPIOB_AFRH |=  (0x6 << (1 * 4));   // Set AF6 (0110) for PB9
    // Configure pull-up resistors for PB8 and PB9
    GPIOB_PUPDR &= ~(3 << (8 * 2));   // Clear pull-up/pull-down for PB8
    GPIOB_PUPDR |= (1 << (8 * 2));    // Enable pull-up for PB8

    GPIOB_PUPDR &= ~(3 << (9 * 2));  // Clear pull-up/pull-down for PB9
    GPIOB_PUPDR |= (1 << (9 * 2));   // Enable pull-up for PB9

    // Configure I2C1 for 100 kHz communication
    I2C1_TIMINGR = (1 << 28) | (9 << 20) | (9 << 16) | (40 << 8) | (48 << 0); 
    I2C1_CR1 |= (1 << 0);       // Enable I2C1
}

// Send a string over UART2
void UART_SendString(char *str)
{
    while (*str)
    {
        while (!(USART2_ISR & (1 << 7)));  // Wait until TX buffer is empty
        USART2_TDR = *str++;  // Transmit character
    }
}

// Simple delay function (busy-wait)
void delay(uint32_t ms)
{
    for (uint32_t i = 0; i < ms * 8000; i++);  // Simple busy-wait delay
}
int I2C_WaitForTransferComplete(void)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (!(I2C1_ISR & (1 << 6)))
    {
        if (--timeout == 0) // Decrement timeout and check if it reaches zero
        {
            UART_SendString("I2C Timeout\r\n");
            return -1; // Indicate timeout error
        }
    }
    return 0; // Indicate success
}
int I2C_WaitForTransferComplet(void)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (!(I2C1_ISR & (1 << 6)))
    {
        if (--timeout == 0) // Decrement timeout and check if it reaches zero
        {
            UART_SendString("I2C Timeout\r\n");
            return -1; // Indicate timeout error
        }
    }
    return 0; // Indicate success
}

// Configure ADS1115 using I2C
void configureADS1115(void)
{
    UART_SendString("Configuring ADS1115...\r\n");

      uint8_t config[3] = {CONFIG_REG & ADS1X15_REG_POINTER_MASK, 0xC3, 0x83};

    // Wait until I2C is ready
    while (I2C1_CR2 & (1 << 25));  // Ensure no ongoing communication
    UART_SendString("I2C Ready...\r\n");

    // Check if I2C bus is busy
    if (I2C1_ISR & (1 << 15))  // BUSY flag
    {
        UART_SendString("I2C Bus is busy. Exiting...\r\n");
        return;
    }

    // Send config to ADS1115
    I2C1_CR2 = (ADS1115_ADDRESS << 1) | (3 << 16) | (1 << 13);  // Set address and write mode
    I2C1_CR2 |= (1 << 25);  // Start communication

    while (!(I2C1_ISR & (1 << 1)));  // Wait for TX ready
    UART_SendString("TX Ready 1...\r\n");

    I2C1_TXDR = config[0];  // Send the register pointer (CONFIG_REG)
if (I2C_WaitForTransferComplete() == -1) return;
    // Check for NACK (Not Acknowledge)
    if (I2C1_ISR & (1 << 4))  // NACKF
    {
        UART_SendString("NACK Received! Stopping communication...\r\n");
        I2C1_CR2 |= (1 << 14);  // Stop communication
        return;
    }
	 if (I2C_WaitForTransferComplete() == -1) return;

    while (!(I2C1_ISR & (1 << 1)));  // Wait for TX ready
    UART_SendString("TX Ready 2...\r\n");

    I2C1_TXDR = config[1];  // Send MSB
	 if (I2C_WaitForTransferComplete() == -1) return;

    while (!(I2C1_ISR & (1 << 1)));  // Wait for TX ready
    UART_SendString("TX Ready 3...\r\n");

    I2C1_TXDR = config[2];  // Send LSB
		

    // Check for NACK again
    if (I2C1_ISR & (1 << 4))  // NACKF
    {
        UART_SendString("NACK Received! Stopping communication...\r\n");
        I2C1_CR2 |= (1 << 14);  // Stop communication
        return;
    }

    while (!(I2C1_ISR & (1 << 6)));  // Wait for transfer complete
    UART_SendString("Transfer Complete...\r\n");

    I2C1_CR2 |= (1 << 14);           // Stop communication
    UART_SendString("Configuration Done...\r\n");
}


// Read data from ADS1115 using I2C
int16_t readADS1115(void)
{
    uint8_t reg = CONVERSION_REG & ADS1X15_REG_POINTER_MASK; // Ensure you're sending the correct register address
    uint8_t data[2];

    // Point to conversion register
    I2C1_CR2 = (ADS1115_ADDRESS << 1) | (1 << 16) | (1 << 13); // Set address for read
    I2C1_CR2 |= (1 << 25);  // Start communication

    // Wait for the TXE flag (Transmit Data Register Empty)
    while (!(I2C1_ISR & (1 << 1)));  // Wait for TX ready

    // Send the register pointer
    I2C1_TXDR = reg;

    // Wait for transfer complete or handle errors
    if (I2C1_ISR & (1 << 3)) { // Check for NACK
        UART_SendString("NACK received after sending reg.\r\n");
        return -1; // Indicate error
    }
 if (I2C_WaitForTransferComplet() == -1) 
    while (!(I2C1_ISR & (1 << 6)));  // Wait for transfer complete

    // Now read from the conversion register
    I2C1_CR2 = (ADS1115_ADDRESS << 1) | (1 << 16) | (1 << 13);  // Set address for read
    I2C1_CR2 |= (1 << 25);  // Start communication

    // Wait for the RX ready
    while (!(I2C1_ISR & (1 << 2)));  // Wait for RX ready
    data[0] = I2C1_RXDR;  // Read MSB
    while (!(I2C1_ISR & (1 << 2)));  // Wait for RX ready
    data[1] = I2C1_RXDR;  // Read LSB

    while (!(I2C1_ISR & (1 << 6)));  // Wait for transfer complete
    I2C1_CR2 |= (1 << 14);           // Stop communication

    return (data[0] << 8) | data[1]; // Return the combined result
}



// Read the sensor and calculate O2 percentage
void readOOM202(void)
{
    // Read ADC value from ADS1115
    adc_OOM202 = readADS1115(); // Assuming you already have this function for reading ADS1115
    
    // Convert ADC value to millivolts (mV)
    mV_OOM202 = (adc_OOM202 * 4.096) / 32767.0;  // Convert ADC to voltage
    mV_OOM202 = mV_OOM202 * 1000;               // Convert voltage to millivolts (mV)

    // Calculate O2 percentage
    current_O2Percent = (mV_OOM202 / mV_baselineO2) * ambientAir_O2Percent;

    // Prepare buffer for printing
    char buffer[100];
    
    // Format the output as: "Analog Value: X mV    O2 Percentage: Y %"
    sprintf(buffer, "Analog Value: %.2f mV    O2 Percentage: %.2f %%\r\n", mV_OOM202, current_O2Percent);

    // Send formatted string over UART (or any communication interface you're using)
    UART_SendString(buffer);  
}
