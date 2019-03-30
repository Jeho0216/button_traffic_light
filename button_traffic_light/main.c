/*
 * button_traffic_light.c
 *
 * Created: 2019-03-30 토 오후 1:53:51
 * Author : 이제호
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

unsigned char h_light[4] = {0x10, 0x40, 0xA0, 0xC0};
unsigned char v_light[4] = {0x08, 0x02, 0x05, 0x03};		//녹 -> 황 -> 적/화 -> 황
unsigned char delay_time[4] = {50, 10, 30, 10};

void delay_100(){
	_delay_ms(100);
}

void traffic_light(uint8_t *number, uint8_t *direction){

	if(*direction == 0){		//수평방향 신호일 경우,
		PORTD = h_light[*number] | 0x01;
		PORTF = h_light[*number] | 0x01;
	}
	else if(*direction == 1){		//수직방향 신호일 경우,
		PORTD = v_light[*number] | 0x80;
		PORTF = v_light[*number] | 0x80;
	}
}

uint8_t button_read(uint8_t *prev, uint8_t *current){
	uint8_t switch_val = PINA & 0x06;
	*current = 0;
	if(switch_val == 0x00){		//버튼 1번이 눌렸을 경우,
		_delay_ms(20);
		*current = 1;
		if((*current == 1) && (*prev == 0)){		//버튼이 눌려있고, 이전상태가 0인경우,
			return 1;
		}
	}
	else if(switch_val == 0x06){	//버튼 2번이 눌렸을 경우,
		_delay_ms(20);
		*current = 1;
		if((*current == 1) && (*prev == 0)){
			return 2;
		}
	}
	return 0;		//아무것도 눌리지 않았을 경우,
}

int main(void){
	DDRA = 0x00;
	DDRD = 0xFF;
	DDRF = 0xFF;
	DDRC = 0x01;		//수동버튼 동작 테스트용.
	PORTA = 0x0FF;
	
	uint8_t button_value = 0;
	uint8_t btn_prev = 0, btn_current = 0;
	uint8_t light_number = 0, light_dir = 0;		//신호 순서, 출력 방향 저장 변수. 출력방향 -> 0 : 수평신호, 1 : 수직신호.
	uint8_t mode = 0;		//자동,수동모드 결정. 0 : 자동, 1 : 수동.
	int i = 0;

	while (1) {
		if(mode == 0){		//자동모드
			for(; light_number < 4; light_number++){
				traffic_light(&light_number, &light_dir);		//신호등 상태, 신호등 방향으로 신호등 점등.
				for(i = 0; i < delay_time[light_number]; i++){	//각 신호등 상태마다 설정된 delay 수행.
					button_value = button_read(&btn_prev, &btn_current);		//자동모드에서 중간에 버튼 입력을 받기 위함.
					btn_prev = btn_current;
					if(button_value == 1)
					break;									//딜레이 도중, 1번 버튼 눌리면 수동모드로 전환.
					delay_100();
				}
				if(button_value == 1){		//2중 반복문 break;
					mode ^= 0x01;		//모드 변경
					break;
				}
			}
			if(button_value != 1){		//모드변경 버튼이 눌리지 않았을 경우에만 실행하기 위함.
				light_number = 0;
				light_dir ^= 0x01;
			}
		}

		else if(mode == 1){		//수동모드
			button_value = button_read(&btn_prev, &btn_current);
			btn_prev = btn_current;
			if(button_value == 1){		//모드 선택버튼이 눌렸을 경우,
				mode ^= 0x01;
			}
			else if(button_value == 2){		//수동조작 버튼이 눌렸을 경우,
				light_number++;
				if(light_number > 3){		//신호등 패턴을 초과했을 경우,
					light_number = 0;
					light_dir ^= 0x01;		//수직, 수평 방향 전환
				}
				traffic_light(&light_number, &light_dir);
			}
		}
		PORTC = mode;		//모드 상태 LED로 출력
	}
}