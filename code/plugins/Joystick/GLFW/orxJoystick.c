/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxJoystick.c
 * @date 26/06/2010
 * @author iarwain@orx-project.org
 *
 * GLFW joystick plugin implementation
 *
 * @todo
 */


#include "orxPluginAPI.h"

#include "GL/glfw.h"

#ifndef __orxEMBEDDED__
  #ifdef __orxMSVC__
    #pragma message("!!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!")
  #else /* __orxMSVC__ */
    #warning !!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of SDL!
  #endif /* __orxMSVC__ */
#endif /* __orxEMBEDDED__ */


/** Module flags
 */
#define orxJOYSTICK_KU32_STATIC_FLAG_NONE     0x00000000 /**< No flags */

#define orxJOYSTICK_KU32_STATIC_FLAG_READY    0x00000001 /**< Ready flag */

#define orxJOYSTICK_KU32_STATIC_MASK_ALL      0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal joystick info structure
 */
typedef struct __orxJOYSTICK_INFO_t
{
  float             afAxisInfoList[orxJOYSTICK_AXIS_SINGLE_NUMBER];
  orxBOOL           bIsConnected;
  unsigned char     au8ButtonInfoList[orxJOYSTICK_BUTTON_SINGLE_NUMBER];

} orxJOYSTICK_INFO;

/** Static structure
 */
typedef struct __orxJOYSTICK_STATIC_t
{
  orxU32            u32Flags;
  orxJOYSTICK_INFO  astJoyInfoList[GLFW_JOYSTICK_LAST + 1];

} orxJOYSTICK_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxJOYSTICK_STATIC sstJoystick;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Event handler
 */
static void orxFASTCALL orxJoystick_GLFW_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  int i;

  /* For all joysticks */
  for(i = 0; i <= GLFW_JOYSTICK_LAST; i++)
  {
    /* Is connected? */
    if(glfwGetJoystickParam(i, GLFW_PRESENT) != GL_FALSE)
    {
      /* Gets axes values */
      glfwGetJoystickPos(i, sstJoystick.astJoyInfoList[i].afAxisInfoList, orxJOYSTICK_AXIS_SINGLE_NUMBER);

      /* Updates connection status */
      sstJoystick.astJoyInfoList[i].bIsConnected = orxTRUE;

      /* Gets button values */
      glfwGetJoystickButtons(i, sstJoystick.astJoyInfoList[i].au8ButtonInfoList, orxJOYSTICK_BUTTON_SINGLE_NUMBER);
    }
    else
    {
      /* Clears info */
      orxMemory_Zero(sstJoystick.astJoyInfoList, (GLFW_JOYSTICK_LAST + 1) * sizeof(orxJOYSTICK_INFO));
    }
  }

  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxJoystick_GLFW_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    /* Is GLFW window opened? */
    if(glfwGetWindowParam(GLFW_OPENED) != GL_FALSE)
    {
      orxCLOCK *pstClock;

      /* Gets core clock */
      pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

      /* Valid? */
      if(pstClock != orxNULL)
      {
        /* Registers event update function */
        eResult = orxClock_Register(pstClock, orxJoystick_GLFW_Update, orxNULL, orxMODULE_ID_MOUSE, orxCLOCK_PRIORITY_LOWEST);
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Updates status */
        sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;
      }
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_GLFW_Exit()
{
  /* Was initialized? */
  if(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    /* Unregisters update function */
    orxClock_Unregister(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxJoystick_GLFW_Update);

    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }

  return;
}

orxFLOAT orxFASTCALL orxJoystick_GLFW_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxU32    u32ID;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Gets ID */
  u32ID = (orxU32)_eAxis / orxJOYSTICK_AXIS_SINGLE_NUMBER;

  /* Is ID valid? */
  if(u32ID <= (orxU32)GLFW_JOYSTICK_LAST)
  {
    /* Plugged? */
    if(sstJoystick.astJoyInfoList[u32ID].bIsConnected != orxFALSE)
    {
      orxS32 s32Axis;

      /* Gets axis */
      s32Axis = _eAxis % orxJOYSTICK_AXIS_SINGLE_NUMBER;
    
      /* Updates result */
      fResult = orx2F(sstJoystick.astJoyInfoList[u32ID].afAxisInfoList[s32Axis]);
    }
    else
    {
      /* Logs message */
      orxLOG("Requested joystick ID <%ld> is not connected.", u32ID);

      /* Updates result */
      fResult = orxFLOAT_0;
    }
  }
  else
  {
    /* Logs message */
    orxLOG("Requested joystick ID <%ld> is out of range.", u32ID);

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxJoystick_GLFW_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  orxU32  u32ID;
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Gets ID */
  u32ID = (orxU32)_eButton / orxJOYSTICK_BUTTON_SINGLE_NUMBER;

  /* Is ID valid? */
  if(u32ID <= (orxU32)GLFW_JOYSTICK_LAST)
  {
    /* Plugged? */
    if(sstJoystick.astJoyInfoList[u32ID].bIsConnected != orxFALSE)
    {
      orxS32 s32Button;

      /* Gets button */
      s32Button = _eButton % orxJOYSTICK_BUTTON_SINGLE_NUMBER;
    
      /* Updates result */
      bResult = (sstJoystick.astJoyInfoList[u32ID].au8ButtonInfoList[s32Button] != GLFW_RELEASE) ? orxTRUE : orxFALSE;
    }
    else
    {
      /* Logs message */
      orxLOG("Requested joystick ID <%ld> is not connected.", u32ID);

      /* Updates result */
      bResult = orxFALSE;
    }
  }
  else
  {
    /* Logs message */
    orxLOG("Requested joystick ID <%ld> is out of range.", u32ID);

    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(JOYSTICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_Init, JOYSTICK, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_Exit, JOYSTICK, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_GetAxisValue, JOYSTICK, GET_AXIS_VALUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_GLFW_IsButtonPressed, JOYSTICK, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_END();