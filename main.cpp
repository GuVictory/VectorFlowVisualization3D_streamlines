#include <QApplication>
#include <QVBoxLayout>


#include <vtkStructuredGrid.h>
#include <vtkCellData.h>
#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkLookupTable.h>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkUnstructuredGridGeometryFilter.h>
#include <vtkPointData.h>
#include <vtkPlane.h>
#include <vtkFieldData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkClipDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPointSource.h>
#include <vtkStreamTracer.h>
#include <vtkArrayCalculator.h>
#include <vtkRibbonFilter.h>
#include <utils.h>

int main(int argc, char** argv) {
    // Needed to ensure appropriate OpenGL context is created for VTK rendering.
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);
    auto *widget = new QVTKOpenGLNativeWidget;
    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(widget);

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    widget->setRenderWindow(renderWindow);
    widget->resize(600, 600);

    vtkNew<vtkNamedColors> colors;

    int meshNumber = 0;
    int renderType = 0;
    std::cout << "На какой сетке построить линии тока? (0: rotor, 1: eintritt, 2: diffusor)" << std::endl;
    std::cin >> meshNumber;
    std::cout << "В каком формате построить данные?" << std::endl <<
        "0: только сетка\n"
        "1: сетка с линиями тока\n"
        "2: сетка и сетка с линиями тока\n"
        "3: сетка с линиями тока в разные моменты времени\n"
        "4: супер построение всего" << std::endl;
    std::cin >> renderType;

    switch (renderType) {
        case 0: {
            infoLog("Загружаю сетку...");

            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader->SetFileName(meshes.at(meshNumber).c_str());
            meshReader->Update();

            // Выведем информацию по загруженной сетке
            infoLog("Сетка загружена:");
            std::cout << "Количество узлов: " << meshReader->GetNumberOfPoints() << std::endl;
            std::cout << "Количество ячеек: " << meshReader->GetNumberOfCells() << std::endl;

            // Переменная в которой происходит весь рендер
            vtkNew<vtkRenderer> renderer;
            renderer->SetBackground(colors->GetColor3d("White").GetData());

            /// Отрисуем сначала саму сетку
            vtkNew<vtkPlane> plane;
            plane->SetOrigin(0.0, 0.0, 0.0);
            plane->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper;
            clipper->SetInputConnection(meshReader->GetOutputPort());
            clipper->SetClipFunction(plane.Get());

            vtkNew<vtkDataSetMapper> wireMapper;
            wireMapper->SetInputConnection(clipper->GetOutputPort());
            wireMapper->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor;
            wireActor->SetMapper(wireMapper.Get());
            wireActor->GetProperty()->SetRepresentationToWireframe();
            wireActor->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor->GetProperty()->SetLineWidth(0.5);
            wireActor->GetProperty()->SetOpacity(0.1);

            renderer->AddActor(wireActor.Get());

            widget->renderWindow()->AddRenderer(renderer);
            break;
        }
        case 1: {
            infoLog("Загружаю сетку...");
            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader->SetFileName(meshes.at(meshNumber).c_str());
            meshReader->Update();

            // Выведем информацию по загруженной сетке
            infoLog("Сетка загружена:");
            std::cout << "Количество узлов: " << meshReader->GetNumberOfPoints() << std::endl;
            std::cout << "Количество ячеек: " << meshReader->GetNumberOfCells() << std::endl;

            // Переменная в которой происходит весь рендер
            vtkNew<vtkRenderer> renderer;
            renderer->SetBackground(colors->GetColor3d("White").GetData());

            /// Отрисуем сначала саму сетку
            vtkNew<vtkPlane> plane;
            plane->SetOrigin(0.0, 0.0, 0.0);
            plane->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper;
            clipper->SetInputConnection(meshReader->GetOutputPort());
            clipper->SetClipFunction(plane.Get());

            vtkNew<vtkDataSetMapper> wireMapper;
            wireMapper->SetInputConnection(clipper->GetOutputPort());
            wireMapper->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor;
            wireActor->SetMapper(wireMapper.Get());
            wireActor->GetProperty()->SetRepresentationToWireframe();
            wireActor->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor->GetProperty()->SetLineWidth(0.5);
            wireActor->GetProperty()->SetOpacity(0.1);

            renderer->AddActor(wireActor.Get());

            /// Отрисуем лини тока
            infoLog("Выполняю расчет для построения линий тока...");
            // Сгенерируем стартовые точки для линий тока
            vtkNew<vtkPointSource> pointSource;
            pointSource->SetCenter(0.0, 0.0, 0.0);
            pointSource->SetRadius(0.3);
            pointSource->SetDistributionToUniform();
            pointSource->SetNumberOfPoints(3000);

            // Установим, что направление расчета происходит как в положительном так и в отрицательном направлении
            // Установим метод для расчета точек линии тока - Рунге Кутты 4го порядка
            vtkNew<vtkStreamTracer> tracer;
            tracer->SetInputConnection(clipper->GetOutputPort());
            tracer->SetSourceConnection(pointSource->GetOutputPort());
            tracer->SetIntegrationDirectionToBoth();
            tracer->SetIntegratorTypeToRungeKutta45();

            vtkNew<vtkArrayCalculator> magCalc;
            magCalc->SetInputConnection(tracer->GetOutputPort());
            magCalc->AddVectorArrayName("Velocity");
            magCalc->SetResultArrayName("MagVelocity");
            magCalc->SetFunction("mag(Velocity)");

            magCalc->Update();
            double magVelocityRange[2];
            magCalc->GetDataSetOutput()->GetPointData()->GetArray("MagVelocity")->GetRange(magVelocityRange);

            // Отрисуем не просто линии, а цилендрическии поверхности определенного радиуса
            vtkNew<vtkRibbonFilter> ribbonFilter;
            ribbonFilter->SetInputConnection(magCalc->GetOutputPort());
            ribbonFilter->SetWidth(0.001);

            vtkNew<vtkPolyDataMapper> streamlineMapper;
            streamlineMapper->SetInputConnection(ribbonFilter->GetOutputPort());
            streamlineMapper->SelectColorArray("MagVelocity");
            streamlineMapper->SetScalarRange(magVelocityRange);

            vtkNew<vtkActor> streamlineActor;
            streamlineActor->SetMapper(streamlineMapper.Get());
            infoLog("Расчет для построения линий тока закончен");

            renderer->AddActor(streamlineActor.Get());
            /// Отрисовка линий тока закончена

            widget->renderWindow()->AddRenderer(renderer);
            break;
        }
        case 2: {

            infoLog("Загружаю сетку...");

            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader->SetFileName(meshes.at(meshNumber).c_str());
            meshReader->Update();

            // Выведем информацию по загруженной сетке
            infoLog("Сетка загружена:");
            std::cout << "Количество узлов: " << meshReader->GetNumberOfPoints() << std::endl;
            std::cout << "Количество ячеек: " << meshReader->GetNumberOfCells() << std::endl;

            double leftViewport[4] = {0.0, 0.0, 0.5, 1.0};
            double rightViewport[4] = {0.5, 0.0, 1.0, 1.0};


            // Переменная в которой происходит весь рендер
            vtkNew<vtkRenderer> leftRenderer;
            leftRenderer->SetViewport(leftViewport);

            vtkNew<vtkRenderer> rightRenderer;
            rightRenderer->SetViewport(rightViewport);


            /// Отрисуем сначала саму сетку
            vtkNew<vtkPlane> plane;
            plane->SetOrigin(0.0, 0.0, 0.0);
            plane->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper;
            clipper->SetInputConnection(meshReader->GetOutputPort());
            clipper->SetClipFunction(plane.Get());

            vtkNew<vtkDataSetMapper> wireMapper;
            wireMapper->SetInputConnection(clipper->GetOutputPort());
            wireMapper->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor;
            wireActor->SetMapper(wireMapper.Get());
            wireActor->GetProperty()->SetRepresentationToWireframe();
            wireActor->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor->GetProperty()->SetLineWidth(0.5);
            wireActor->GetProperty()->SetOpacity(0.1);

            leftRenderer->AddActor(wireActor.Get());
            rightRenderer->AddActor(wireActor.Get());


            /// Отрисуем лини тока
            infoLog("Выполняю расчет для построения линий тока...");
            // Сгенерируем стартовые точки для линий тока
            vtkNew<vtkPointSource> pointSource;
            pointSource->SetCenter(0.0, 0.0, 0.0);
            pointSource->SetRadius(0.3);
            pointSource->SetDistributionToUniform();
            pointSource->SetNumberOfPoints(3000);

            // Установим, что направление расчета происходит как в положительном так и в отрицательном направлении
            // Установим метод для расчета точек линии тока - Рунге Кутты 4го порядка
            vtkNew<vtkStreamTracer> tracer;
            tracer->SetInputConnection(clipper->GetOutputPort());
            tracer->SetSourceConnection(pointSource->GetOutputPort());
            tracer->SetIntegrationDirectionToBoth();
            tracer->SetIntegratorTypeToRungeKutta45();

            vtkNew<vtkArrayCalculator> magCalc;
            magCalc->SetInputConnection(tracer->GetOutputPort());
            magCalc->AddVectorArrayName("Velocity");
            magCalc->SetResultArrayName("MagVelocity");
            magCalc->SetFunction("mag(Velocity)");

            magCalc->Update();
            double magVelocityRange[2];
            magCalc->GetDataSetOutput()->GetPointData()->GetArray("MagVelocity")->GetRange(magVelocityRange);

            // Отрисуем не просто линии, а цилендрическии поверхности определенного радиуса
            vtkNew<vtkRibbonFilter> ribbonFilter;
            ribbonFilter->SetInputConnection(magCalc->GetOutputPort());
            ribbonFilter->SetWidth(0.001);

            vtkNew<vtkPolyDataMapper> streamlineMapper;
            streamlineMapper->SetInputConnection(ribbonFilter->GetOutputPort());
            streamlineMapper->SelectColorArray("MagVelocity");
            streamlineMapper->SetScalarRange(magVelocityRange);

            vtkNew<vtkActor> streamlineActor;
            streamlineActor->SetMapper(streamlineMapper.Get());
            infoLog("Расчет для построения линий тока закончен");

            rightRenderer->AddActor(streamlineActor.Get());

            /// Отрисовка линий тока закончена
            leftRenderer->SetBackground(colors->GetColor3d("White").GetData());
            rightRenderer->SetBackground(colors->GetColor3d("White").GetData());

            widget->renderWindow()->AddRenderer(leftRenderer);
            widget->renderWindow()->AddRenderer(rightRenderer);
            break;
        }
        case 3: {
            infoLog("Загружаю первую сетку...");
            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader1 = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader1->SetFileName(meshes.at(meshNumber).c_str());
            meshReader1->Update();
            infoLog("Первая сетка загружена");

            infoLog("Загружаю вторую сетку...");
            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader2 = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader2->SetFileName(meshes.at(meshNumber + 3).c_str());
            meshReader2->Update();
            infoLog("Вторая сетка загружена");

            infoLog("Загружаю третью сетку...");
            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader3 = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader3->SetFileName(meshes.at(meshNumber + 6).c_str());
            meshReader3->Update();
            infoLog("Третья сетка загружена");


            double topLeftViewport[4] = {0.0, 0.5, 0.5, 1.0};
            double topRightViewport[4] = {0.5, 0.5, 1.0, 1.0};
            double bottomLeftViewport[4] = {0.0, 0.0, 0.5, 0.5};
            double bottomRightViewport[4] = {0.5, 0.0, 1.0, 0.5};


            // Переменная в которой происходит весь рендер
            vtkNew<vtkRenderer> topLeftRenderer;
            topLeftRenderer->SetViewport(topLeftViewport);

            vtkNew<vtkRenderer> topRightRenderer;
            topRightRenderer->SetViewport(topRightViewport);

            // Переменная в которой происходит весь рендер
            vtkNew<vtkRenderer> bottomLeftRenderer;
            bottomLeftRenderer->SetViewport(bottomLeftViewport);

            vtkNew<vtkRenderer> bottomRightRenderer;
            bottomRightRenderer->SetViewport(bottomRightViewport);


            /// Отрисуем сначала саму сетку
            vtkNew<vtkPlane> plane1;
            plane1->SetOrigin(0.0, 0.0, 0.0);
            plane1->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper1;
            clipper1->SetInputConnection(meshReader1->GetOutputPort());
            clipper1->SetClipFunction(plane1.Get());

            vtkNew<vtkDataSetMapper> wireMapper1;
            wireMapper1->SetInputConnection(clipper1->GetOutputPort());
            wireMapper1->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor1;
            wireActor1->SetMapper(wireMapper1.Get());
            wireActor1->GetProperty()->SetRepresentationToWireframe();
            wireActor1->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor1->GetProperty()->SetLineWidth(0.5);
            wireActor1->GetProperty()->SetOpacity(0.1);

            vtkNew<vtkPlane> plane2;
            plane2->SetOrigin(0.0, 0.0, 0.0);
            plane2->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper2;
            clipper2->SetInputConnection(meshReader2->GetOutputPort());
            clipper2->SetClipFunction(plane2.Get());

            vtkNew<vtkDataSetMapper> wireMapper2;
            wireMapper2->SetInputConnection(clipper2->GetOutputPort());
            wireMapper2->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor2;
            wireActor2->SetMapper(wireMapper2.Get());
            wireActor2->GetProperty()->SetRepresentationToWireframe();
            wireActor2->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor2->GetProperty()->SetLineWidth(0.5);
            wireActor2->GetProperty()->SetOpacity(0.1);

            vtkNew<vtkPlane> plane3;
            plane3->SetOrigin(0.0, 0.0, 0.0);
            plane3->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper3;
            clipper3->SetInputConnection(meshReader3->GetOutputPort());
            clipper3->SetClipFunction(plane3.Get());

            vtkNew<vtkDataSetMapper> wireMapper3;
            wireMapper3->SetInputConnection(clipper3->GetOutputPort());
            wireMapper3->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor3;
            wireActor3->SetMapper(wireMapper3.Get());
            wireActor3->GetProperty()->SetRepresentationToWireframe();
            wireActor3->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor3->GetProperty()->SetLineWidth(0.5);
            wireActor3->GetProperty()->SetOpacity(0.1);

            topLeftRenderer->AddActor(wireActor1.Get());
            topRightRenderer->AddActor(wireActor1.Get());
            bottomLeftRenderer->AddActor(wireActor2.Get());
            bottomRightRenderer->AddActor(wireActor3.Get());


            /// Отрисуем лини тока
            infoLog("Выполняю расчет для построения линий тока на первой сетке...");
            // Сгенерируем стартовые точки для линий тока
            vtkNew<vtkPointSource> pointSource1;
            pointSource1->SetCenter(0.0, 0.0, 0.0);
            pointSource1->SetRadius(0.3);
            pointSource1->SetDistributionToUniform();
            pointSource1->SetNumberOfPoints(3000);

            // Установим, что направление расчета происходит как в положительном так и в отрицательном направлении
            // Установим метод для расчета точек линии тока - Рунге Кутты 4го порядка
            vtkNew<vtkStreamTracer> tracer1;
            tracer1->SetInputConnection(clipper1->GetOutputPort());
            tracer1->SetSourceConnection(pointSource1->GetOutputPort());
            tracer1->SetIntegrationDirectionToBoth();
            tracer1->SetIntegratorTypeToRungeKutta45();

            vtkNew<vtkArrayCalculator> magCalc1;
            magCalc1->SetInputConnection(tracer1->GetOutputPort());
            magCalc1->AddVectorArrayName("Velocity");
            magCalc1->SetResultArrayName("MagVelocity");
            magCalc1->SetFunction("mag(Velocity)");

            magCalc1->Update();
            double magVelocityRange1[2];
            magCalc1->GetDataSetOutput()->GetPointData()->GetArray("MagVelocity")->GetRange(magVelocityRange1);

            // Отрисуем не просто линии, а цилендрическии поверхности определенного радиуса
            vtkNew<vtkRibbonFilter> ribbonFilter1;
            ribbonFilter1->SetInputConnection(magCalc1->GetOutputPort());
            ribbonFilter1->SetWidth(0.001);

            vtkNew<vtkPolyDataMapper> streamlineMapper1;
            streamlineMapper1->SetInputConnection(ribbonFilter1->GetOutputPort());
            streamlineMapper1->SelectColorArray("MagVelocity");
            streamlineMapper1->SetScalarRange(magVelocityRange1);

            vtkNew<vtkActor> streamlineActor1;
            streamlineActor1->SetMapper(streamlineMapper1.Get());
            infoLog("Расчет для построения линий тока в первый момент времени закончен закончен");

            topRightRenderer->AddActor(streamlineActor1.Get());

            /// Отрисовка линий тока закончена

            /// Отрисуем лини тока
            infoLog("Выполняю расчет для построения линий тока на второй сетке...");
            // Сгенерируем стартовые точки для линий тока
            vtkNew<vtkPointSource> pointSource2;
            pointSource2->SetCenter(0.0, 0.0, 0.0);
            pointSource2->SetRadius(0.3);
            pointSource2->SetDistributionToUniform();
            pointSource2->SetNumberOfPoints(3000);

            // Установим, что направление расчета происходит как в положительном так и в отрицательном направлении
            // Установим метод для расчета точек линии тока - Рунге Кутты 4го порядка
            vtkNew<vtkStreamTracer> tracer2;
            tracer2->SetInputConnection(clipper2->GetOutputPort());
            tracer2->SetSourceConnection(pointSource2->GetOutputPort());
            tracer2->SetIntegrationDirectionToBoth();
            tracer2->SetIntegratorTypeToRungeKutta45();

            vtkNew<vtkArrayCalculator> magCalc2;
            magCalc2->SetInputConnection(tracer2->GetOutputPort());
            magCalc2->AddVectorArrayName("Velocity");
            magCalc2->SetResultArrayName("MagVelocity");
            magCalc2->SetFunction("mag(Velocity)");

            magCalc2->Update();
            double magVelocityRange2[2];
            magCalc2->GetDataSetOutput()->GetPointData()->GetArray("MagVelocity")->GetRange(magVelocityRange2);

            // Отрисуем не просто линии, а цилендрическии поверхности определенного радиуса
            vtkNew<vtkRibbonFilter> ribbonFilter2;
            ribbonFilter2->SetInputConnection(magCalc2->GetOutputPort());
            ribbonFilter2->SetWidth(0.001);

            vtkNew<vtkPolyDataMapper> streamlineMapper2;
            streamlineMapper2->SetInputConnection(ribbonFilter2->GetOutputPort());
            streamlineMapper2->SelectColorArray("MagVelocity");
            streamlineMapper2->SetScalarRange(magVelocityRange2);

            vtkNew<vtkActor> streamlineActor2;
            streamlineActor2->SetMapper(streamlineMapper2.Get());
            infoLog("Расчет для построения линий тока во второй момент времени закончен");

            bottomLeftRenderer->AddActor(streamlineActor2.Get());

            /// Отрисовка линий тока закончена

            /// Отрисуем лини тока
            infoLog("Выполняю расчет для построения линий тока на третьей сетке...");
            // Сгенерируем стартовые точки для линий тока
            vtkNew<vtkPointSource> pointSource3;
            pointSource3->SetCenter(0.0, 0.0, 0.0);
            pointSource3->SetRadius(0.3);
            pointSource3->SetDistributionToUniform();
            pointSource3->SetNumberOfPoints(3000);

            // Установим, что направление расчета происходит как в положительном так и в отрицательном направлении
            // Установим метод для расчета точек линии тока - Рунге Кутты 4го порядка
            vtkNew<vtkStreamTracer> tracer3;
            tracer3->SetInputConnection(clipper3->GetOutputPort());
            tracer3->SetSourceConnection(pointSource3->GetOutputPort());
            tracer3->SetIntegrationDirectionToBoth();
            tracer3->SetIntegratorTypeToRungeKutta45();

            vtkNew<vtkArrayCalculator> magCalc3;
            magCalc3->SetInputConnection(tracer3->GetOutputPort());
            magCalc3->AddVectorArrayName("Velocity");
            magCalc3->SetResultArrayName("MagVelocity");
            magCalc3->SetFunction("mag(Velocity)");

            magCalc3->Update();
            double magVelocityRange3[2];
            magCalc3->GetDataSetOutput()->GetPointData()->GetArray("MagVelocity")->GetRange(magVelocityRange3);

            // Отрисуем не просто линии, а цилендрическии поверхности определенного радиуса
            vtkNew<vtkRibbonFilter> ribbonFilter3;
            ribbonFilter3->SetInputConnection(magCalc3->GetOutputPort());
            ribbonFilter3->SetWidth(0.001);

            vtkNew<vtkPolyDataMapper> streamlineMapper3;
            streamlineMapper3->SetInputConnection(ribbonFilter3->GetOutputPort());
            streamlineMapper3->SelectColorArray("MagVelocity");
            streamlineMapper3->SetScalarRange(magVelocityRange3);

            vtkNew<vtkActor> streamlineActor3;
            streamlineActor3->SetMapper(streamlineMapper3.Get());
            infoLog("Расчет для построения линий тока в третий момент времени закончен");

            bottomRightRenderer->AddActor(streamlineActor3.Get());

            /// Отрисовка линий тока закончена
            topLeftRenderer->SetBackground(colors->GetColor3d("White").GetData());
            topRightRenderer->SetBackground(colors->GetColor3d("White").GetData());
            bottomLeftRenderer->SetBackground(colors->GetColor3d("White").GetData());
            bottomRightRenderer ->SetBackground(colors->GetColor3d("White").GetData());

            widget->renderWindow()->AddRenderer(topLeftRenderer);
            widget->renderWindow()->AddRenderer(topRightRenderer);
            widget->renderWindow()->AddRenderer(bottomLeftRenderer);
            widget->renderWindow()->AddRenderer(bottomRightRenderer);

            break;
        }
        case 4: {
            infoLog("Загружаю первую сетку...");
            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader1 = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader1->SetFileName(meshes.at(0).c_str());
            meshReader1->Update();
            infoLog("Первая сетка загружена");

            infoLog("Загружаю вторую сетку...");
            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader2 = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader2->SetFileName(meshes.at(1).c_str());
            meshReader2->Update();
            infoLog("Вторая сетка загружена");

            infoLog("Загружаю третью сетку...");
            // Считаем сетку
            vtkSmartPointer<vtkXMLUnstructuredGridReader> meshReader3 = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
            meshReader3->SetFileName(meshes.at(2).c_str());
            meshReader3->Update();
            infoLog("Третья сетка загружена");


            double leftViewport[4] = {0.0, 0.0, 0.5, 1.0};
            double rightViewport[4] = {0.5, 0.0, 1.0, 1.0};


            // Переменная в которой происходит весь рендер
            vtkNew<vtkRenderer> leftRenderer;
            leftRenderer->SetViewport(leftViewport);

            vtkNew<vtkRenderer> rightRenderer;
            rightRenderer->SetViewport(rightViewport);


            /// Отрисуем сначала саму сетку
            vtkNew<vtkPlane> plane1;
            plane1->SetOrigin(0.0, 0.0, 0.0);
            plane1->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper1;
            clipper1->SetInputConnection(meshReader1->GetOutputPort());
            clipper1->SetClipFunction(plane1.Get());

            vtkNew<vtkDataSetMapper> wireMapper1;
            wireMapper1->SetInputConnection(clipper1->GetOutputPort());
            wireMapper1->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor1;
            wireActor1->SetMapper(wireMapper1.Get());
            wireActor1->GetProperty()->SetRepresentationToWireframe();
            wireActor1->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor1->GetProperty()->SetLineWidth(0.5);
            wireActor1->GetProperty()->SetOpacity(0.1);

            vtkNew<vtkPlane> plane2;
            plane2->SetOrigin(0.0, 0.0, 0.0);
            plane2->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper2;
            clipper2->SetInputConnection(meshReader2->GetOutputPort());
            clipper2->SetClipFunction(plane2.Get());

            vtkNew<vtkDataSetMapper> wireMapper2;
            wireMapper2->SetInputConnection(clipper2->GetOutputPort());
            wireMapper2->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor2;
            wireActor2->SetMapper(wireMapper2.Get());
            wireActor2->GetProperty()->SetRepresentationToWireframe();
            wireActor2->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor2->GetProperty()->SetLineWidth(0.5);
            wireActor2->GetProperty()->SetOpacity(0.1);

            vtkNew<vtkPlane> plane3;
            plane3->SetOrigin(0.0, 0.0, 0.0);
            plane3->SetNormal(0.0, 0.0, 0.0);
            vtkNew<vtkClipDataSet> clipper3;
            clipper3->SetInputConnection(meshReader3->GetOutputPort());
            clipper3->SetClipFunction(plane3.Get());

            vtkNew<vtkDataSetMapper> wireMapper3;
            wireMapper3->SetInputConnection(clipper3->GetOutputPort());
            wireMapper3->ScalarVisibilityOff();

            vtkNew<vtkActor> wireActor3;
            wireActor3->SetMapper(wireMapper3.Get());
            wireActor3->GetProperty()->SetRepresentationToWireframe();
            wireActor3->GetProperty()->SetColor(0.4, 0.4, 0.4);
            wireActor3->GetProperty()->SetLineWidth(0.5);
            wireActor3->GetProperty()->SetOpacity(0.1);

            leftRenderer->AddActor(wireActor1.Get());
            rightRenderer->AddActor(wireActor1.Get());

            leftRenderer->AddActor(wireActor2.Get());
            rightRenderer->AddActor(wireActor2.Get());

            leftRenderer->AddActor(wireActor3.Get());
            rightRenderer->AddActor(wireActor3.Get());


            /// Отрисуем лини тока
            infoLog("Выполняю расчет для построения линий тока на первой сетке...");
            // Сгенерируем стартовые точки для линий тока
            vtkNew<vtkPointSource> pointSource1;
            pointSource1->SetCenter(0.0, 0.0, 0.0);
            pointSource1->SetRadius(0.3);
            pointSource1->SetDistributionToUniform();
            pointSource1->SetNumberOfPoints(3000);

            // Установим, что направление расчета происходит как в положительном так и в отрицательном направлении
            // Установим метод для расчета точек линии тока - Рунге Кутты 4го порядка
            vtkNew<vtkStreamTracer> tracer1;
            tracer1->SetInputConnection(clipper1->GetOutputPort());
            tracer1->SetSourceConnection(pointSource1->GetOutputPort());
            tracer1->SetIntegrationDirectionToBoth();
            tracer1->SetIntegratorTypeToRungeKutta45();

            vtkNew<vtkArrayCalculator> magCalc1;
            magCalc1->SetInputConnection(tracer1->GetOutputPort());
            magCalc1->AddVectorArrayName("Velocity");
            magCalc1->SetResultArrayName("MagVelocity");
            magCalc1->SetFunction("mag(Velocity)");

            magCalc1->Update();
            double magVelocityRange1[2];
            magCalc1->GetDataSetOutput()->GetPointData()->GetArray("MagVelocity")->GetRange(magVelocityRange1);

            // Отрисуем не просто линии, а цилендрическии поверхности определенного радиуса
            vtkNew<vtkRibbonFilter> ribbonFilter1;
            ribbonFilter1->SetInputConnection(magCalc1->GetOutputPort());
            ribbonFilter1->SetWidth(0.001);

            vtkNew<vtkPolyDataMapper> streamlineMapper1;
            streamlineMapper1->SetInputConnection(ribbonFilter1->GetOutputPort());
            streamlineMapper1->SelectColorArray("MagVelocity");
            streamlineMapper1->SetScalarRange(magVelocityRange1);

            vtkNew<vtkActor> streamlineActor1;
            streamlineActor1->SetMapper(streamlineMapper1.Get());
            infoLog("Расчет для построения линий тока на первой сетке закончен");

            rightRenderer->AddActor(streamlineActor1.Get());

            /// Отрисовка линий тока закончена

            /// Отрисуем лини тока
            infoLog("Выполняю расчет для построения линий тока на второй сетке...");
            // Сгенерируем стартовые точки для линий тока
            vtkNew<vtkPointSource> pointSource2;
            pointSource2->SetCenter(0.0, 0.0, 0.0);
            pointSource2->SetRadius(0.3);
            pointSource2->SetDistributionToUniform();
            pointSource2->SetNumberOfPoints(3000);

            // Установим, что направление расчета происходит как в положительном так и в отрицательном направлении
            // Установим метод для расчета точек линии тока - Рунге Кутты 4го порядка
            vtkNew<vtkStreamTracer> tracer2;
            tracer2->SetInputConnection(clipper2->GetOutputPort());
            tracer2->SetSourceConnection(pointSource2->GetOutputPort());
            tracer2->SetIntegrationDirectionToBoth();
            tracer2->SetIntegratorTypeToRungeKutta45();

            vtkNew<vtkArrayCalculator> magCalc2;
            magCalc2->SetInputConnection(tracer2->GetOutputPort());
            magCalc2->AddVectorArrayName("Velocity");
            magCalc2->SetResultArrayName("MagVelocity");
            magCalc2->SetFunction("mag(Velocity)");

            magCalc2->Update();
            double magVelocityRange2[2];
            magCalc2->GetDataSetOutput()->GetPointData()->GetArray("MagVelocity")->GetRange(magVelocityRange2);

            // Отрисуем не просто линии, а цилендрическии поверхности определенного радиуса
            vtkNew<vtkRibbonFilter> ribbonFilter2;
            ribbonFilter2->SetInputConnection(magCalc2->GetOutputPort());
            ribbonFilter2->SetWidth(0.001);

            vtkNew<vtkPolyDataMapper> streamlineMapper2;
            streamlineMapper2->SetInputConnection(ribbonFilter2->GetOutputPort());
            streamlineMapper2->SelectColorArray("MagVelocity");
            streamlineMapper2->SetScalarRange(magVelocityRange2);

            vtkNew<vtkActor> streamlineActor2;
            streamlineActor2->SetMapper(streamlineMapper2.Get());
            infoLog("Расчет для построения линий тока на второй сетке закончен");

            rightRenderer->AddActor(streamlineActor2.Get());

            /// Отрисовка линий тока закончена

            /// Отрисуем лини тока
            infoLog("Выполняю расчет для построения линий тока на третьей сетке...");
            // Сгенерируем стартовые точки для линий тока
            vtkNew<vtkPointSource> pointSource3;
            pointSource3->SetCenter(0.0, 0.0, 0.0);
            pointSource3->SetRadius(0.3);
            pointSource3->SetDistributionToUniform();
            pointSource3->SetNumberOfPoints(3000);

            // Установим, что направление расчета происходит как в положительном так и в отрицательном направлении
            // Установим метод для расчета точек линии тока - Рунге Кутты 4го порядка
            vtkNew<vtkStreamTracer> tracer3;
            tracer3->SetInputConnection(clipper3->GetOutputPort());
            tracer3->SetSourceConnection(pointSource3->GetOutputPort());
            tracer3->SetIntegrationDirectionToBoth();
            tracer3->SetIntegratorTypeToRungeKutta45();

            vtkNew<vtkArrayCalculator> magCalc3;
            magCalc3->SetInputConnection(tracer3->GetOutputPort());
            magCalc3->AddVectorArrayName("Velocity");
            magCalc3->SetResultArrayName("MagVelocity");
            magCalc3->SetFunction("mag(Velocity)");

            magCalc3->Update();
            double magVelocityRange3[2];
            magCalc3->GetDataSetOutput()->GetPointData()->GetArray("MagVelocity")->GetRange(magVelocityRange3);

            // Отрисуем не просто линии, а цилендрическии поверхности определенного радиуса
            vtkNew<vtkRibbonFilter> ribbonFilter3;
            ribbonFilter3->SetInputConnection(magCalc3->GetOutputPort());
            ribbonFilter3->SetWidth(0.001);

            vtkNew<vtkPolyDataMapper> streamlineMapper3;
            streamlineMapper3->SetInputConnection(ribbonFilter3->GetOutputPort());
            streamlineMapper3->SelectColorArray("MagVelocity");
            streamlineMapper3->SetScalarRange(magVelocityRange3);

            vtkNew<vtkActor> streamlineActor3;
            streamlineActor3->SetMapper(streamlineMapper3.Get());
            infoLog("Расчет для построения линий тока на третьей сетке закончен");

            rightRenderer->AddActor(streamlineActor3.Get());

            /// Отрисовка линий тока закончена
            leftRenderer->SetBackground(colors->GetColor3d("White").GetData());
            rightRenderer->SetBackground(colors->GetColor3d("White").GetData());

            widget->renderWindow()->AddRenderer(leftRenderer);
            widget->renderWindow()->AddRenderer(rightRenderer);

            break;
        }
    }

    widget->renderWindow()->SetWindowName("StreamLines");

    widget->show();
    app.exec();
    return EXIT_SUCCESS;
}
