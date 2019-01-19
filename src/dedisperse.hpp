
/*******************************************************************
*FUNCTION: AdvancedAnalysis::calculateFullDM(char* timeFlags,char* freqFlags)
*char* timeFlags	:Time samples marked 1 are ignored (or clipped).
*char* freqFlags	:Channels marked 1 are ignored (or clipped).
*Calculates the dedispersed time series.
*If time or channel or both filtering are turned off the corresponding
*arrays have all 0.
*******************************************************************/
void AdvancedAnalysis::calculateFullDM(char* timeFlags,char* freqFlags)
{
	
	float* ptrRawData=rawData;
	char* ptrTimeFlags=timeFlags;
	char* ptrFreqFlags;
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;

	for(long int i=0;i<length;i++,ptrTimeFlags++)
	{
		ptrRawData+=startChannel;
		ptrFreqFlags=freqFlags;	
			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFreqFlags++)
			{

				pos=i+delayTable[j];	//shift to correct for dispersion.
				if(!(*ptrFreqFlags)&!(*ptrTimeFlags))
				{
					fullDM[pos]+=(*ptrRawData);
					count[pos]++;
				}
				else
				{
					fullDMUnfiltered[pos]+=(*ptrRawData);
					countUnfiltered[pos]++;
				}
			}
		
		ptrRawData+=endExclude;
	}
	float* ptrFullDM=fullDM;
	float* ptrFullDMUnfiltered=fullDMUnfiltered;
	int* ptrCount=count;
	int* ptrCountUnfiltered=countUnfiltered;
	

	for(int i=0;i<length+maxDelay;i++,ptrFullDM++,ptrFullDMUnfiltered++,ptrCount++,ptrCountUnfiltered++)
	{

		(*ptrFullDMUnfiltered)+=(*ptrFullDM);
		(*ptrCountUnfiltered)+=(*ptrCount);		
	}        

}
/*******************************************************************
*FUNCTION: AdvancedAnalysis::calculateFullDM(unsigned short int* filteredRawData)
*unsigned short int* filteredRawData - filtered raw data
*Calculates the dedispersed time series using the filtered raw data.
*******************************************************************/
void AdvancedAnalysis::calculateFullDM(unsigned short int* filteredRawData)
{
	

	
	long int pos;

	int startChannel=info.startChannel;
	int stopChannel=info.stopChannel;
	int totalChan=info.noOfChannels;
	int endExclude=info.noOfChannels-stopChannel;
	float* ptrRawData=rawData;
	unsigned short int* ptrFilteredRawData=filteredRawData;
	for(long int i=0;i<length;i++)
	{
			ptrFilteredRawData+=startChannel;	
			ptrRawData+=startChannel;	
			for(int j=startChannel;j<stopChannel;j++,ptrRawData++,ptrFilteredRawData++)
			{

				pos=i+delayTable[j];	//shift to correct for dispersion.
				
				fullDM[pos]+=(*ptrFilteredRawData);
				count[pos]++;
				
				
				fullDMUnfiltered[pos]+=(*ptrRawData)*32768;
				countUnfiltered[pos]++;
			}
			ptrRawData+=endExclude;
			ptrFilteredRawData+=endExclude;
			
	}
}
void AdvancedAnalysis::mergeExcess(float* excess_,int* countExcess_,float* excessUnfiltered_,int* countExcessUnfiltered_)
{
	float* ptrFullDM=fullDM;
	float* ptrFullDMUnfiltered=fullDMUnfiltered;
	int* ptrCount=count;
	int* ptrCountUnfiltered=countUnfiltered;
	float* ptrExcess=excess_;
	float* ptrExcessUnfiltered=excessUnfiltered_;
	int* ptrCountExcess=countExcess_;
	int* ptrCountExcessUnfiltered=countExcessUnfiltered_;
	if(excess_)
	{
		for(int i=0;i<maxDelay;i++,ptrFullDM++,ptrFullDMUnfiltered++,ptrExcess++,ptrExcessUnfiltered++,ptrCount++,ptrCountUnfiltered++,ptrCountExcess++,ptrCountExcessUnfiltered++)
		{
			(*ptrFullDM)+=(*ptrExcess);
			(*ptrFullDMUnfiltered)+=(*ptrExcessUnfiltered);
			(*ptrCount)+=(*ptrCountExcess);
			(*ptrCountUnfiltered)+=(*ptrCountExcessUnfiltered);		
		}
	}
	memcpy(excess,&fullDM[length],maxDelay*sizeof(float));
	memcpy(excessUnfiltered,&fullDMUnfiltered[length],maxDelay*sizeof(float));
	memcpy(countExcess,&count[length],maxDelay*sizeof(int));	
	memcpy(countExcessUnfiltered,&countUnfiltered[length],maxDelay*sizeof(int));
}
