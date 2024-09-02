//**********************************************************************************************************************
// FileName : edgeDetect.h
// FilePath : utility/
// Author   : Christian Marty
// Date		: 24.09.2018
// Website  : www.christian-marty.ch
//**********************************************************************************************************************
#ifndef EDGE_DETECT_H_
#define EDGE_DETECT_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct{
	bool oldState;
}edgeDetect_t;

//**********************************************************************************************************************
// Initialize state
//
// Return: -
//**********************************************************************************************************************
static inline void ed_init(edgeDetect_t *edgeDetect, bool signal)
{
	edgeDetect->oldState = signal;
}

//**********************************************************************************************************************
// Find change in signal
//
// Return: True if signal changes
//**********************************************************************************************************************
static inline bool ed_onChange(edgeDetect_t *edgeDetect, bool signal)
{
	if(edgeDetect->oldState == signal) return false;
	
	edgeDetect->oldState = signal;
	return true;
}

//**********************************************************************************************************************
// Find falling edge in signal
//
// Return: True if signal has falling edge
//**********************************************************************************************************************
static inline bool ed_onFalling(edgeDetect_t *edgeDetect, bool signal)
{
	if(edgeDetect->oldState == signal) return false;
	edgeDetect->oldState = signal;
	
	if(signal != false) return false;

	return true;
}

//**********************************************************************************************************************
// Find rising edge in signal
//
// Return: True if signal has rising edge
//**********************************************************************************************************************
static inline bool ed_onRising(edgeDetect_t *edgeDetect, bool signal)
{
	if(edgeDetect->oldState == signal) return false;
	edgeDetect->oldState = signal;
	
	if(signal != true) return false;
	
	return true;
}

#ifdef __cplusplus
	}
#endif

#endif /* EDGE_DETECT_H_ */