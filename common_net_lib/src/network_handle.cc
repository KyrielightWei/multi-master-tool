
#include"network_handle.h"


/*************************************************************
 *      Class SimplePacket<T>
 * ***********************************************************
*/
template<typename T>
void * SimplePacket<T>::get_val_ptr()
{
    return (void *)&value;
}

template<typename T>
void SimplePacket<T>::set_val(const T & val)
{
    value = val;
}

template<typename T>
unsigned int SimplePacket<T>::get_packet_size()
{
    return sizeof(T);
}


/*************************************************************
 *      
 * ***********************************************************
*/