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


#ifndef ia_segmentador_abdominal_Activator_h
#define ia_segmentador_abdominal_Activator_h

#include <ctkPluginActivator.h>

namespace mitk
{
  class ia_segmentador_abdominal_Activator : public QObject, public ctkPluginActivator
  {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "ia_segmentador_abdominal")
    Q_INTERFACES(ctkPluginActivator)

  public:
    void start(ctkPluginContext *context);
    void stop(ctkPluginContext *context);

  }; // ia_segmentador_abdominal_Activator
}

#endif // ia_segmentador_abdominal_Activator_h
