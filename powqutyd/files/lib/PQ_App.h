/****************************************************************************\
**                   _        _____ _               _                      **
**                  / \      | ____| |__   ___ _ __| | ___                 **
**                 / _ \     |  _| | '_ \ / _ \ '__| |/ _ \                **
**                / ___ \ _  | |___| |_) |  __/ |  | |  __/                **
**               /_/   \_(_) |_____|_.__/ \___|_|  |_|\___|                **
**                                                                         **
*****************************************************************************
** ID:                $Id: PQ_App.h 109 2016-03-24 10:40:35Z fabian.haussel $
** Last committed:    $Revision: 109 $
** Last changed by:   $Author: fabian.haussel $
** Last changed date: $Date: 2016-03-24 11:40:35 +0100 (Thu, 24 Mar 2016) $
\****************************************************************************/

/*************************************************************************//**
* @file    PQ_App.h
* @brief   Power Quality C implementation library interface
* @details Headerfile containing defines, structures and functions declarations
*          required to successfully run the A.Eberle Power Quality library.
* @author  F.Haussel Fabian.Haussel@a-eberle.de
*
* @project{WeSense}
*
* @moduleacronym{pqLibrary}
*
* @date 28.01.2015: First implementation
*
* @version $Revision: 109 $
****************************************************************************/

#ifndef PQ_APP_H_INCLUDED
#define PQ_APP_H_INCLUDED


/*! \defgroup pqLibApi Power Quality Library Interface
 *  @{
 */

/* --------------- includes --------------- */

/* --------------- defines ---------------- */
#define LIB_VERSION_LENGTH (9)  ///< Defining the length of the library version string. DO NOT CHANGE!
#define MAX_FRAMESIZE   (2048)  ///< Defining the size of the time signal input buffer. Can be used outside to allocate the input buffer. DO NOT CHANGE!

/* ---------- opaque declarations --------- */
typedef struct _powerQualityInstance PQInstance; ///< Opaque declaration of power quality library structure.
typedef PQInstance* pPQInstance;                 ///< Opaque declaration of power quality library structure pointer.

/* ------------- declarations ------------- */
/*! @enum  PQ_ERROR
 *  @brief Definition of library return values.
 *  @details Some of the defined functions return error values to indicate correct or incorrect processing.
 *           They are grouped together in this enumeration. All errors are fatal. After an error occurs
 *           the processing with the used library instance shall immediately be stopped!
 */
typedef enum {
    PQ_NO_ERROR = 0,           ///< No error: normal processing.
    PQ_MEM_ERROR,              ///< Memory allocation failed.
    PQ_INVALED_CONFIG_ERROR,   ///< Error during creation / configuration of library
    PQ_HANDLE_ERROR,           ///< Invalid handle given to functions.
    PQ_PROCESSING_ERROR        ///< Error during processing.
}PQ_ERROR;


/*! @enum  PQ_EVENT_TYPE
 *  @brief Definition of power quality event identifiers.
 *  @details The library is able to detect power quality events. Each event type has its own identifier
 *           defined in this enumeration.
 */
typedef enum {
    PQ_EVENT_TYPE_NO,        ///< No Event. Used to signal empty event in PQEvent array of library output structure PQResult.
    PQ_EVENT_TYPE_DIP,       ///< Event indicating a voltage dip of 10% - 90% of the reference voltage on the measurment signal.
    PQ_EVENT_TYPE_SWELL,     ///< Event indicating a voltage swell > 110% of the reference voltage on the measurment signal.
    PQ_EVENT_TYPE_INTERRUPT, ///< Event indicating a voltage dip < 10% of the reference voltage on the measurment signal.
    PQ_EVENT_TYPE_HARMONIC   ///< Event indicating that > 5% of the measured values of one specific harmonic are over the defined threshold.
}PQ_EVENT_TYPE;


/*! @struct PQConfig
 *  @brief  Configuration structure.
 *  @details The struct contains hardware- and system specific parameters required for setting up a valid power quality library instance at create time.
 *           It needs to be filled by the library user and handed to function createPowerQuality().
 *           One of the elements depends on define HIGH_SAMPLERATE_ENABLE set in src/PQ_config.h. If the define is set the library expects a input signal with 10240Hz, else 2048Hz.
 *           The define and samplerate of the configuration structure have to match. For details please see struct fields description.
 */
typedef struct {
    int sampleRate;  ///< Samplerate of time input signal: 2048 or 10240 Hz depending on define HIGH_SAMPLERATE_ENABLE.
    float HW_offset; ///< Time input signal offset correction value from hardware.
    float HW_scale;  ///< Time input signal scaling factor from hardware.
}PQConfig, *pPQConfig;


/*! @struct PQInfo
 *  @brief  Information structure.
 *  @details The (empty) structure is handed to function createPowerQuality(). Upon successfull return the structure
 *           contains all settings of the created instance that may be valuable outside the library (for the user or support team in case of problems).
 *           In case of the function returning with an error the structure may not be filled with valid data.
 */
typedef struct {
    int sampleRate;                           /*!< Samplerate of the input signal which the library was configured for. */
    int framesize;                            /*!< Number of samples of input signal required for one call of applyPowerQuality().
                                                   Depending on samplerate: 2048Hz - 448 Samples; 10240 - 2048 Samples. */
    int blocksize;                            /*!< Defines the number of samples which were transfered with one data package from the charger-hardware.
                                                   This value is samplerate dependend and hardcoded internally 2048Hz - 28 Samples; 10240 - 64 Samples. */
    int nmbBlocksRegulation;                  /*!< Defines the number of blocks until the internal timpstamp compensation is recursively adapted. */
    char library_version[LIB_VERSION_LENGTH]; /*!< Contains the library version number in string format.
                                                   Shall be used to indentify the version of the library if support is needed. */
}PQInfo, *pPQInfo;


/*! @struct PQEvent
 *  @brief  Power Quality Event structure.
 *  @details An array of this structures is part of the PQResult structure to hold the events occured on the signal processed with the last applyPowerQuality call.
 *           One instance of this structure contains all required values to fully describe an event. Not all events use the same elements
 *           of the structure. Elements encapsulated with DEBUG_PRINT are for testing purposes only.
 */
typedef struct {
    PQ_EVENT_TYPE type;   /*!<  Event type. For details please see PQ_EVENT_TYPE */
    long long startTime;  /*!<  Start time of the event in ms (UTC) */
#ifdef DEBUG_PRINT
    float startTimeFract; /*!<  Fraction of a millisecond of the start time. */
#endif // DEBUG_PRINT
    int length;           /*!<  Length of the event in ms (UTC) */
#ifdef DEBUG_PRINT
    float lengthFract;    /*!<  Fraction of a millisecond of the event length. */
#endif
    float minMax;         /*!<  Min/Max Voltage during event. Only valid for PQ_EVENT_TYPE_DIP, PQ_EVENT_TYPE_SWELL, PQ_EVENT_TYPE_INTERRUPT. */
    int harmonic_number;  /*!<  Number of the Harmonic which causes the event. Only valid for PQ_EVENT_TYPE_HARMONIC. */
    int fail_percentage;  /*!<  Percentage of measured harmonic values exceeding the threshold for harmonic specific in harmonic_number. Only valid for PQ_EVENT_TYPE_HARMONIC. */
} PQEvent, *pPQEvent;


/*! @struct PQResult
 *  @brief  Power Quality Results structure.
 *  @details The (empty) structure is handed to function applyPowerQuality(). Upon successfull return the structure
 *           contains the valid power quality measurment values for all quantities.
 */

/*!
*  Example define structures:
*  \snippet framework.c Create Library Structures
*/

typedef struct {
    float PowerVoltageEff_1012T[2];    ///< 10/12T RMS Voltage values (unit: V)
    char  PowerVoltage1012TExist[2];   ///< Has PowerVoltageEff_1012T a valid value (0, 1)
    float PowerVoltageEff_5060T;       ///< 50/60T RMS Voltage values (unit: V)
    char  PowerVoltageEff5060TExist;   ///< Has PowerVoltageEff_5060T a valid value (0, 1)
    float PowerFrequency1012T[2];      ///< 10/12T frequency value (unit: Hz)
    char  PowerFrequency1012TExist[2]; ///< Has PowerFrequency 10/12T a valid value (0, 1)
    float PowerFrequency5060T;         ///< 50/60T  frequency value (unit: Hz)
    char  PowerFrequency5060TExist;    ///< Has PowerFrequency 50/60T a valid value (0, 1)
    float Harmonics[7];                ///< Harmonic output H3, H5, H7, H9, H11, H13, H15 (unit: % of reference voltage)
    char  HarmonicsExist;              ///< Has Harmonics valid values (0, 1)

    long long timeStamp1012T[2];       ///< Timestamp of the 10/12T values in ms (UTC)
#ifdef DEBUG_PRINT
    float timeStampFrac1012T[2];       ///< Fractional timestamp of the 10/12T values in ms
#endif // DEBUG_PRINT

    long long timeStamp5060T;          ///< Timestamp of the 50/60T values in ms (UTC)
#ifdef DEBUG_PRINT
    float timeStampFract5060T;         ///< Fractional timestamp of the 50/60T values in ms
#endif // DEBUG_PRINT

    /* pq screen stuff */
    float voltage_percent;             ///< 10/12T rms voltage value (unit: % of reference voltage)
    float harmonicsFailPercent[7];     ///< Number of measured values above threshold for odd harmonics H3 - H15 (unit: %)
    int   referenceVoltage;            ///< Reference Voltage used in library (110V or 230V) (unit: V)

    PQEvent pqEvents[40];              ///< Array holding the events occured on the time signal processed with applyPowerQuality call.

    int nmbPqEvents;                   ///< Number of power quality events.

}PQResult, *pPQResult;

/* --------- function declarations -------- */

/*************************************************************************//**
* @fn      PQ_ERROR createPowerQuality(const pPQConfig const pPQConf,
*                            pPQInstance* const ppPQ,
*                            pPQInfo const pPQInf)
* @brief   Creation of a Power Quality library instance.
* @details This function gets a library configuration structure containing all required
*          parameters to set up one instance of the power quality library.
*          Upon successfull return the second parameter contains the created
*          library instance. In addition the function fills the handed over
*          info structure with all information required outside the library.
*
* @param[in] pPQConf
*          Configuration structure containing all required setup parameters.
* @param[out] ppPQ
*          double pointer to an PQ instance structure
*          contains address of power quality instance after successful
*          return of the function
* @param[out] pPQInf
*          A pointer to an empty info structure to be filled during function
*          processing.
*
* @return  PQ_NO_ERROR after successful processing;
*          else one of the other error codes from PQ_ERROR;
*
* Example function call:
* \snippet framework.c Create Library Instance
*****************************************************************************/

PQ_ERROR createPowerQuality(const pPQConfig const pPQConf,
                            pPQInstance* const ppPQ,
                            pPQInfo const pPQInf);


/*************************************************************************//**
* @fn      applyPowerQuality(pPQInstance const pPQ,
*                           const float* pIn,
*                           pPQResult const pResult,
*                           float* const  pOut,
*                           const long long* const timeStamps,
*                           const int nTimeStamps)
* @brief   Data processing.
* @details The function applies the power quality algorithms to the input signal
*          generating the values for all output quantities. Upon return all
*          generated values can be found in parameter pResult. The function expects
*          a specific number of samples in pIn. For details please see PQInfo::framesize.
*          The scaling of the ADC input samples is done within the library.
*          To make the scaled samples available outside the library (e.g. for
*          recording/storing) they are saved within parameter pOut. If the function is called
*          with parameter pOut = NULL (see example), no output is generated.
*          Also the function expects to get one timestamp for every sample block
*          provided by the hardware device. The user can calculate the number of timestamps required by:
*          nTimeStamps = PQInfo::framesize / PQInfo::blocksize.
*
* @param[in] pPQ
*          Pointer to a correctly initialized PQ instance structure.
* @param[in] pIn
*          Pointer to the time signal to be processed.
* @param[out] pResult
*          Pointer to the structure holding the calculation results.
* @param[in, out] pOut
*          Pointer to the address where the scaled input values shall be stored.
* @param[in] timeStamps
*          Pointer to the time stamp values in ms.
* @param[in] nTimeStamps
*          Number of time stamps handed in with timeStamps parameter.
*
* @return  PQ_NO_ERROR after successful processing;
*          else one of the other defined error codes;
*
* Example function call:
* \snippet framework.c Apply Power Quality Library
*****************************************************************************/
PQ_ERROR applyPowerQuality(pPQInstance const pPQ,
                           const float* pIn,
                           pPQResult const pResult,
                           float* const  pOut,
                           const long long* const timeStamps,
#ifdef DEBUG_PRINT
                           const float* const timeStampsFract,
#endif // DEBUG_PRINT
                           const int nTimeStamps);

/* ****************************************************************************** */
/* function: destroyPowerQuality                                                  */
/*                                                                                */
/* description:                                                                   */
/* deletion of one PQ instance                                                    */
/*                                                                                */
/* parameter:                                                                     */
/* ppPQ:            input; double pointer to an PQ instance structure             */
/*                  contains NULL after successful return of function             */
/*                                                                                */
/* return value:    none                                                          */
/* ****************************************************************************** */

/*************************************************************************//**
* @fn      void destroyPowerQuality(pPQInstance* const ppPQ)
* @brief   Delete pq library instance.
* @details The function receives a double pointer to a power quality library structure
*          instance, frees all allocated memory including all submodules
*          and writes NULL to the pointer.
*
* @param[in,out] ppPQ
*          Double pointer to a pq library instance structure.
*          Contains NULL after successful return of function.
*
* @return  None
*
* Example function call:
* \snippet framework.c Destroy Library Instance
*****************************************************************************/
void destroyPowerQuality(pPQInstance* const ppPQ);

/*! @} */

#endif // PQ_APP_H_INCLUDED
