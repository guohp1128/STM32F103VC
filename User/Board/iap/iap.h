// * @file
// * @brief   
// * @author  
// * @version V1.0
// * @date    2022/10/26
// *******************************************************************************
// * @note
// * 
// * @attention
// * 
// *******************************************************************************
// * @par 修改日志:
// * <table>
// * <tr><th>Date        <th>Version  <th>Author(ID:xxxxxx)  <th>Description</tr>
// * <tr><td>2022/10/26 <td>V1.0     <td>	(ID:123456)  <td>创建初始版本</tr>
// * </table>



#ifndef IAP_H_
#define IAP_H_


/* includes *******************************************************************/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

#ifdef __cplusplus
extern "C" {
#endif

/**Exported typedef************************************************************/
/* USER CODE BEGIN ET */

/* USER CODE END ET */


/**Exported constants**********************************************************/
/* USER CODE BEGIN EC */
#define UPGRADE_SUCCESS             0x00
#define UPGRADE_FAIL                0x01
#define UPGRADE_NONE                0xFF


/* USER CODE END EC */


/**Exported macro**************************************************************/
/* USER CODE BEGIN EM */

/* USER CODE END EM */


/**Exported functions prototypes***********************************************/
/* USER CODE BEGIN EFP */
extern char UpgradeApp(void); //升级应用程序(APP)
extern void JumpToApp(void); //跳转到应用程序(APP)
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* IAP_H_ */






