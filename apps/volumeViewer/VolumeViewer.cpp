//
//                 INTEL CORPORATION PROPRIETARY INFORMATION
//
//    This software is supplied under the terms of a license agreement or
//    nondisclosure agreement with Intel Corporation and may not be copied
//    or disclosed except in accordance with the terms of that agreement.
//    Copyright (C) 2014 Intel Corporation. All Rights Reserved.
//

#include <algorithm>
#include "VolumeViewer.h"
#include "TransferFunctionEditor.h"
#include "SliceWidget.h"

VolumeViewer::VolumeViewer(const std::vector<std::string> &filenames) : renderer(NULL), transferFunction(NULL), osprayWindow(NULL), autoRotationRate(0.025f) {

    //! Create and configure the OSPRay state.
    initObjects(filenames);

    //! Default window size.
    resize(1024, 768);

    //! Create an OSPRay window and set it as the central widget, but don't let it start rendering until we're done with setup.
    osprayWindow = new QOSPRayWindow(renderer);  setCentralWidget(osprayWindow);

    //! Set the window bounds based on the OSPRay world bounds (always [(0,0,0), (1,1,1)) for volumes).
    osprayWindow->setWorldBounds(osp::box3f(osp::vec3f(0.0f), osp::vec3f(1.0f)));

    //! Configure the user interface widgets and callbacks.
    initUserInterfaceWidgets();

    //! Show the window.
    show();

}

void VolumeViewer::autoRotate(bool set) {

    if(osprayWindow == NULL)
        return;

    if(autoRotateAction != NULL)
        autoRotateAction->setChecked(set);

    if(set) {
        osprayWindow->setRotationRate(autoRotationRate);
        osprayWindow->updateGL();
    }
    else {
        osprayWindow->setRotationRate(0.);
    }
}

void VolumeViewer::addSlice(std::string filename) {

    //! Create a slice widget and add it to the dock. This widget modifies the slice directly.
    SliceWidget * sliceWidget = new SliceWidget(models, osp::box3f(osp::vec3f(0.0f), osp::vec3f(1.0f)));
    connect(sliceWidget, SIGNAL(sliceChanged()), this, SLOT(render()));
    sliceWidgetsLayout.addWidget(sliceWidget);

    //! Load state from file if specified.
    if(!filename.empty())
        sliceWidget->load(filename);
}



}

void VolumeViewer::importObjectsFromFile(const std::string &filename) {

    //! Create an OSPRay model.
    OSPModel model = ospNewModel();

    //! Load OSPRay objects from a file.
    OSPObjectCatalog catalog = ospImportObjects(filename.c_str());

    //! For now we set the same transfer function on all volumes.
    for (size_t i=0 ; catalog->entries[i] ; i++) if (catalog->entries[i]->type == OSP_VOLUME) ospSetObject(catalog->entries[i]->object, "transferFunction", transferFunction);

    //! Add the loaded volume(s) to the model.
    for (size_t i=0 ; catalog->entries[i] ; i++) if (catalog->entries[i]->type == OSP_VOLUME) ospAddVolume(model, (OSPVolume) catalog->entries[i]->object);

    //! Keep vector of all loaded volume(s).
    for (size_t i=0 ; catalog->entries[i] ; i++) if (catalog->entries[i]->type == OSP_VOLUME) volumes.push_back((OSPVolume) catalog->entries[i]->object);

    //! Commit the OSPRay object state.
    ospCommitCatalog(catalog);  ospCommit(model);  models.push_back(model);

}

void VolumeViewer::initObjects(const std::vector<std::string> &filenames) {

    //! Create an OSPRay renderer.
    renderer = ospNewRenderer("raycast_volume_renderer");

    //! Create an OSPRay transfer function.
    transferFunction = ospNewTransferFunction("piecewise_linear");

    //! Create an OSPRay light source.
    OSPLight light = ospNewLight(NULL, "DirectionalLight");  ospSet3f(light, "direction", 1.0f, -2.0f, -1.0f);  ospSet3f(light, "color", 1.0f, 1.0f, 1.0f);

    //! Set the light source on the renderer.
    ospCommit(light);  ospSetData(renderer, "lights", ospNewData(1, OSP_OBJECT, &light));

    //! Detailed failure information will be reported by OSPRay.
    exitOnCondition(renderer == NULL || transferFunction == NULL, "could not create OSPRay object");  ospCommit(transferFunction);

    //! Load OSPRay objects from files.
    for (size_t i=0 ; i < filenames.size() ; i++) importObjectsFromFile(filenames[i]);

}

void VolumeViewer::initUserInterfaceWidgets() {

    //! Add the "auto rotate" widget and callback.
    QToolBar *toolbar = addToolBar("toolbar");
    autoRotateAction = new QAction("Auto rotate", this);
    autoRotateAction->setCheckable(true);
    connect(autoRotateAction, SIGNAL(toggled(bool)), this, SLOT(autoRotate(bool)));
    toolbar->addAction(autoRotateAction);

    //! Add the "next timestep" widget and callback.
    QAction *nextTimeStepAction = new QAction("Next timestep", this);
    connect(nextTimeStepAction, SIGNAL(triggered()), this, SLOT(nextTimeStep()));
    toolbar->addAction(nextTimeStepAction);

    //! Add the "play timesteps" widget and callback.
    QAction *playTimeStepsAction = new QAction("Play timesteps", this);
    playTimeStepsAction->setCheckable(true);
    connect(playTimeStepsAction, SIGNAL(toggled(bool)), this, SLOT(playTimeSteps(bool)));
    toolbar->addAction(playTimeStepsAction);

    //! Connect the "play timesteps" timer.
    connect(&playTimeStepsTimer, SIGNAL(timeout()), this, SLOT(nextTimeStep()));

    //! Add the "add slice" widget and callback.
    QAction *addSliceAction = new QAction("Add slice", this);
    connect(addSliceAction, SIGNAL(triggered()), this, SLOT(addSlice()));
    toolbar->addAction(addSliceAction);

    //! Create the transfer function editor dock widget, this widget modifies the transfer function directly.
    QDockWidget *transferFunctionEditorDockWidget = new QDockWidget("Transfer Function Editor", this);
    transferFunctionEditor = new TransferFunctionEditor(transferFunction);
    transferFunctionEditorDockWidget->setWidget(transferFunctionEditor);
    connect(transferFunctionEditor, SIGNAL(transferFunctionChanged()), this, SLOT(commitVolumes()));
    connect(transferFunctionEditor, SIGNAL(transferFunctionChanged()), this, SLOT(render()));
    addDockWidget(Qt::LeftDockWidgetArea, transferFunctionEditorDockWidget);

    //! Set the transfer function editor widget to its minimum allowed height, to leave room for other dock widgets.
    transferFunctionEditor->setMaximumHeight(transferFunctionEditor->minimumSize().height());

    //! Create a scrollable dock widget for any added slices.
    QDockWidget *slicesDockWidget = new QDockWidget("Slices", this);
    QScrollArea *slicesScrollArea = new QScrollArea();
    QWidget *slicesWidget = new QWidget();
    slicesWidget->setLayout(&sliceWidgetsLayout);
    slicesScrollArea->setWidget(slicesWidget);
    slicesScrollArea->setWidgetResizable(true);
    slicesDockWidget->setWidget(slicesScrollArea);
    addDockWidget(Qt::LeftDockWidgetArea, slicesDockWidget);
}
