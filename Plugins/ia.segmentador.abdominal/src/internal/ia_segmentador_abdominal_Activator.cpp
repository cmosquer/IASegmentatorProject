/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "ia_segmentador_abdominal_Activator.h"
#include "IASegmentadorAbdominalView.h"

namespace mitk
{
  void ia_segmentador_abdominal_Activator::start(ctkPluginContext *context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(IASegmentadorAbdominalView, context)
  }

  void ia_segmentador_abdominal_Activator::stop(ctkPluginContext *context) { Q_UNUSED(context) }
}
