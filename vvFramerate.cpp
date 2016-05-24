#include "vvFramerate.h"

#include <vtkCoordinate.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <GL/GLContextData.h>

#include <Vrui/Vrui.h>

#include "vvApplicationState.h"
#include "vvContextState.h"

#include <algorithm>
#include <cassert>
#include <sstream>

//------------------------------------------------------------------------------
vvFramerate::DataItem::DataItem()
{
  actor->SetTextScaleModeToViewport();
  vtkCoordinate *coord = actor->GetPositionCoordinate();
  coord->SetCoordinateSystemToNormalizedDisplay();
  coord->SetValue(0.01, 0.99);
}

//------------------------------------------------------------------------------
vvFramerate::vvFramerate()
  : m_visible(false)
{
  m_tprop->SetJustificationToLeft();
  m_tprop->SetVerticalJustificationToTop();
  m_tprop->SetFontSize(8);
  m_tprop->SetBackgroundColor(.25, .25, .25);
  m_tprop->SetBackgroundOpacity(0.5);
}

//------------------------------------------------------------------------------
vvFramerate::~vvFramerate()
{
}

//------------------------------------------------------------------------------
void vvFramerate::initVvContext(vvContextState &vvContext,
                                GLContextData &contextData) const
{
  this->Superclass::initVvContext(vvContext, contextData);

  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);

  dataItem->actor->SetTextProperty(m_tprop.Get());
  vvContext.renderer().AddActor2D(dataItem->actor.GetPointer());
}

//------------------------------------------------------------------------------
void vvFramerate::syncApplicationState(const vvApplicationState &state)
{
  this->Superclass::syncApplicationState(state);

  const size_t FPSCacheSize = 64;

  m_timer.elapse();
  const double time = m_timer.getTime();

  if (time == 0.) // We just started the timer -- first frame.
    {
    return;
    }
  else if (m_times.size() < FPSCacheSize)
    {
    m_times.push_back(time);
    }
  else
    {
    std::rotate(m_times.begin(), m_times.begin() + 1, m_times.end());
    m_times.back() = time;
    }

  // If showing the framerate, trigger another render:
  if (m_visible)
    {
    Vrui::requestUpdate();
    }
}

//------------------------------------------------------------------------------
void vvFramerate::syncContextState(const vvApplicationState &appState,
                                   const vvContextState &contextState,
                                   GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  dataItem->actor->SetVisibility(m_visible ? 1 : 0);
  if (m_visible)
    {
    double time = 0.0;
    for (size_t i = 0; i < m_times.size(); ++i)
      {
      time += m_times[i];
      }
    double fps = time > 1e-5 ? m_times.size() / time : 0.;

    std::ostringstream fpsStr;
    fpsStr << "FPS: " << fps;
    dataItem->actor->SetInput(fpsStr.str().c_str());
    }
}
