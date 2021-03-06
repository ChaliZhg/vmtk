/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkvmtkITKImageToImageFilter_h
#define __vtkvmtkITKImageToImageFilter_h

#include "vtkvmtkITK.h"

// ITK includes
#include <itkCommand.h>
#include <itkProcessObject.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageExport.h>
#include <vtkImageImport.h>
#include <vtkVersion.h>

#undef itkExceptionMacro
#define itkExceptionMacro(x) \
  { \
  ::std::ostringstream message; \
  message << "itk::ERROR: " << this->GetNameOfClass() \
          << "(" << this << "): "; \
  std::cout << message.str().c_str() << std::endl; \
  }

#undef itkGenericExceptionMacro
#define itkGenericExceptionMacro(x) \
  { \
  ::std::ostringstream message; \
  message << "itk::ERROR: " x; \
  std::cout << message.str() << std::endl; \
  }

/// \brief Abstract base class for connecting ITK and VTK.
///
/// vtkvmtkITKImageToImageFilter provides a foo.
class VTK_VMTK_ITK_EXPORT vtkvmtkITKImageToImageFilter
  : public vtkImageAlgorithm
{
public:
  static vtkvmtkITKImageToImageFilter *New()
   {
     return new vtkvmtkITKImageToImageFilter;
   };

  vtkTypeMacro(vtkvmtkITKImageToImageFilter, vtkImageAlgorithm);

  void PrintSelf(ostream& os, vtkIndent indent)
  {
    Superclass::PrintSelf ( os, indent );
    this->vtkExporter->PrintSelf ( os, indent );
    this->vtkImporter->PrintSelf ( os, indent );
  };

  ///
  /// This method considers the sub filters MTimes when computing this objects
  /// modified time.
  unsigned long int GetMTime()
  {
    unsigned long int t1, t2;

    t1 = this->Superclass::GetMTime();
    t2 = this->vtkExporter->GetMTime();
    if (t2 > t1)
      {
      t1 = t2;
      }
    t2 = this->vtkImporter->GetMTime();
    if (t2 > t1)
      {
      t1 = t2;
      }
    return t1;
  };

  ///
  /// Pass modified message to itk filter
  void Modified()
  {
    this->Superclass::Modified();
    if (this->m_Process)
      {
      m_Process->Modified();
      }
  };

  ///
  /// Pass DebugOn.
  void DebugOn()
  {
    this->m_Process->DebugOn();
  };

  ///
  /// Pass DebugOff.
  void DebugOff()
  {
    this->m_Process->DebugOff();
  };

  ///
  /// Pass SetNumberOfThreads.
  void SetNumberOfThreads(int val)
  {
    this->m_Process->SetNumberOfThreads(val);
  };

  ///
  /// Pass SetNumberOfThreads.
  int GetNumberOfThreads()
  {
    return this->m_Process->GetNumberOfThreads();
  };

  ///
  /// This method returns the cache to make a connection
  /// It justs feeds the request to the sub filter.
  virtual void SetOutput ( vtkDataObject* d ) { this->vtkImporter->SetOutput ( d ); };
  virtual vtkImageData *GetOutput() { return this->vtkImporter->GetOutput(); };
  virtual vtkImageData *GetOutput(int idx)
  {
    return (vtkImageData *) this->vtkImporter->GetOutput(idx);
  };

  ///
  /// Set the Input of the filter.
  virtual void SetInput(vtkImageData *Input)
  {
#if (VTK_MAJOR_VERSION <= 5)
    this->vtkCast->SetInput(Input);
#else
    this->vtkCast->SetInputData(Input);
#endif
  };

  virtual void SetInputConnection(vtkAlgorithmOutput* input)
  {
    this->vtkCast->SetInputConnection(input);
  };

  virtual void SetInputConnection(int port, vtkAlgorithmOutput* input)
  {
    this->vtkCast->SetInputConnection(port, input);
  };

  ///
  /// Return the input to the filter
  virtual vtkDataObject* GetInput()
  {
    return (vtkDataObject::SafeDownCast( this->vtkCast->GetInput() ));
  };

  ///  Override vtkSource's Update so that we can access
  /// this class's GetOutput(). vtkSource's GetOutput is not virtual.
#if (VTK_MAJOR_VERSION <= 5)
  void Update()
    {
      if (this->GetOutput(0))
        {
        this->GetOutput(0)->Update();
        if ( this->GetOutput(0)->GetSource() )
          {
          ///          this->SetErrorCode( this->GetOutput(0)->GetSource()->GetErrorCode() );
          }
        }
    }
#else
  virtual void Update()
    {
      this->vtkCast->Update();
      this->vtkImporter->Update();
    }
  virtual void Update(int port)
    {
      this->vtkCast->Update();
      this->vtkImporter->Update(port);
   }
#endif
  void HandleProgressEvent ()
  {
    if ( this->m_Process )
      {
      this->UpdateProgress ( m_Process->GetProgress() );
      }
  };
  void HandleStartEvent ()
  {
    this->InvokeEvent(vtkCommand::StartEvent,NULL);
  };
  void HandleEndEvent ()
  {
    this->InvokeEvent(vtkCommand::EndEvent,NULL);
  };
  /// ETX

 protected:

  /// BTX
  /// Dummy ExecuteData
  void ExecuteData (vtkDataObject *)
  {
    vtkWarningMacro(<< "This filter does not respond to Update(). Doing a GetOutput->Update() instead.");
  }
  /// ETX

  vtkvmtkITKImageToImageFilter()
  {
    /// Need an import, export, and a ITK pipeline
    this->vtkCast = vtkImageCast::New();
    this->vtkExporter = vtkImageExport::New();
    this->vtkImporter = vtkImageImport::New();
#if (VTK_MAJOR_VERSION <= 5)
    this->vtkExporter->SetInput ( this->vtkCast->GetOutput() );
#else
    this->vtkExporter->SetInputConnection( this->vtkCast->GetOutputPort() );
#endif
    this->m_Process = NULL;
    this->m_ProgressCommand = MemberCommand::New();
    this->m_ProgressCommand->SetCallbackFunction ( this, &vtkvmtkITKImageToImageFilter::HandleProgressEvent );
    this->m_StartEventCommand = MemberCommand::New();
    this->m_StartEventCommand->SetCallbackFunction ( this, &vtkvmtkITKImageToImageFilter::HandleStartEvent );
    this->m_EndEventCommand = MemberCommand::New();
    this->m_EndEventCommand->SetCallbackFunction ( this, &vtkvmtkITKImageToImageFilter::HandleEndEvent );
  };
  ~vtkvmtkITKImageToImageFilter()
  {
    vtkDebugMacro ("Destructing vtkvmtkITKImageToImageFilter");
    this->vtkExporter->Delete();
    this->vtkImporter->Delete();
    this->vtkCast->Delete();
  };

  /// BTX
  void LinkITKProgressToVTKProgress ( itk::ProcessObject* process )
  {
    if ( process )
      {
      this->m_Process = process;
      this->m_Process->AddObserver ( itk::ProgressEvent(), this->m_ProgressCommand );
      this->m_Process->AddObserver ( itk::StartEvent(), this->m_StartEventCommand );
      this->m_Process->AddObserver ( itk::EndEvent(), this->m_EndEventCommand );
      }
  };

  typedef itk::SimpleMemberCommand<vtkvmtkITKImageToImageFilter> MemberCommand;
  typedef MemberCommand::Pointer MemberCommandPointer;

  itk::ProcessObject::Pointer m_Process;
  MemberCommandPointer m_ProgressCommand;
  MemberCommandPointer m_StartEventCommand;
  MemberCommandPointer m_EndEventCommand;

  /// ITK Progress object
  /// To/from VTK
  vtkImageCast* vtkCast;
  vtkImageImport* vtkImporter;
  vtkImageExport* vtkExporter;

private:
  vtkvmtkITKImageToImageFilter(const vtkvmtkITKImageToImageFilter&);  /// Not implemented.
  void operator=(const vtkvmtkITKImageToImageFilter&);  /// Not implemented.
};

#endif
