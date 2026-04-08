package ws

const (
	ID_DEVICE uint8 = iota
	ID_HOST
	ID_A
	ID_B
)

const (
	StatusOk uint8 = iota
	StatusErr
	StatusInitial uint8 = 255
)

const (
	CMD_UNKNOWN           uint8 = iota
	CMD_SYSTEM_REBOOT     uint8 = 1
	CMD_SYSTEM_END        uint8 = 19
	CMD_LED_DEFAULT_BEGIN uint8 = 20
	CMD_LED_DEFAULT       uint8 = 21
	CMD_LED_DEFAULT_END   uint8 = 39

	CMD_ADC_BEGIN    uint8 = 40
	CMD_ADC_DMA      uint8 = 41
	CMD_ADC_READ_CH0 uint8 = 42
	CMD_ADC_READ_CH1 uint8 = 43
	CMD_ADC_READ_CH2 uint8 = 44
	CMD_ADC_READ_CH3 uint8 = 45
	CMD_ADC_READ_CH4 uint8 = 46
	CMD_ADC_END      uint8 = 59

	CMD_TB6612FNG_BEGIN       uint8 = 60
	CMD_TB6612FNG_POWER       uint8 = 61
	CMD_TB6612FNG_MOTOR_1     uint8 = 62
	CMD_TB6612FNG_MOTOR_2     uint8 = 63
	CMD_TB6612FNG_MOTOR_STATE uint8 = 64
	CMD_TB6612FNG_END         uint8 = 79
)

const (
	RequestSize  = 8
	ResponseSize = 7
	RxSize       = 64
)
