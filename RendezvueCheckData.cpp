#include "RendezvueCheckData.h"

CRendezvueCheckData::CRendezvueCheckData(void) :
	m_tmp_size(0)
	,m_tmp_buf(NULL)
	,m_tmp_index(0)
	, m_head_find_count(0)
	,m_tail_find_count(0)
{
	m_tmp_size = (IMAGE_WIDTH_VIA_ETH * IMAGE_HEIGHT_VIA_ETH * 3) + 1000;
    m_tmp_buf = (unsigned char *)malloc(sizeof(unsigned char) * m_tmp_size);
	m_tmp_index = 0;

	m_head_string[0] = '[';
	m_head_string[1] = 'R';
	m_head_string[2] = 'D';
	m_head_string[3] = 'V';

	m_tail_string[0] = 'E';
	m_tail_string[1] = 'S';
	m_tail_string[2] = 'B';
	m_tail_string[3] = ']';
}

CRendezvueCheckData::~CRendezvueCheckData(void)
{
	if (m_tmp_buf != NULL) free(m_tmp_buf);
}

unsigned char* CRendezvueCheckData::FindData(unsigned char* input_buf, int input_size, int* out_size)
{	
	for (int i = 0; i < input_size; i++)
	{
		//find head
		if (m_head_find_count < HEAD_LENGTH)
		{
			if (input_buf[i] == m_head_string[m_head_find_count])
			{
				m_head_find_count++;
			}
			else
			{
				init_variable();
			}
		}
		else
		{
			memcpy(m_tmp_buf + m_tmp_index, input_buf+i, input_size-i);
			m_tmp_index += (input_size - i);

			//find tail
			if (m_tmp_index > TAIL_LENGTH)
			{
				//printf("check tail") ;
				if (m_tmp_buf[m_tmp_index - 1] == m_tail_string[3] && 
					m_tmp_buf[m_tmp_index - 2] == m_tail_string[2] && 
					m_tmp_buf[m_tmp_index - 3] == m_tail_string[1] && 
					m_tmp_buf[m_tmp_index - 4] == m_tail_string[0] )
				{
					if( m_tmp_index >= m_head_find_count + TAIL_LENGTH )
					{
						//printf("tail") ;
						//ok
						(*out_size) = m_tmp_index;
						//printf("buf size = %d\n", m_tmp_index) ;
											
						m_head_find_count = 0;
						m_tmp_index = 0;

						return m_tmp_buf;
					}
					else
					{
						init_variable() ;
					}
				}
			}
			break;
		}
	}

	return NULL;
}

void CRendezvueCheckData::init_variable()
{
    m_tmp_index = 0;
    m_head_find_count = 0;
    m_tail_find_count = 0;
}
