#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#include "qtkalmanfilterexperimentdialog.h"

#include <cassert>
#include <cstdlib>
#include <fstream>

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/numeric/ublas/functional.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>

#include "fileio.h"
#include "kalmanfilter.h"
#include "kalmanfilterexample.h"
#include "kalmanfilterexperiment.h"
#include "kalmanfilterexperimentparameter.h"
#include "kalmanfilterexperimentparametertype.h"
#include "kalmanfilterparameter.h"
#include "kalmanfilterparameters.h"
#include "kalmanfiltertypes.h"
#include "matrix.h"
#include "qtkalmanfilterdialog.h"
#include "qtkalmanfilterermodel.h"
#include "qtkalmanfilterexamplesdialog.h"
#include "qtkalmanfilterexperimentmodel.h"
#include "qtkalmanfiltererparameterdialog.h"
#include "qtmatrix.h"
#include "qtstandardkalmanfiltercalculationdialog.h"
#include "qtstdvectorstringmodel.h"
#include "qtsteadystatekalmanfiltercalculationdialog.h"
#include "qtwhitenoisesystemparametersdialog.h"
#include "standardwhitenoisesystemparameters.h"
#include "ui_qtkalmanfilterexperimentdialog.h"
#include "ui_qtwhitenoisesystemparametersdialog.h"
#include "whitenoisesystemparameter.h"
#include "whitenoisesystemtypes.h"

#pragma GCC diagnostic pop

ribi::kalman::QtKalmanFilterExperimentDialog::QtKalmanFilterExperimentDialog(
  const boost::shared_ptr<QtKalmanFilterExperimentModel> model,
  QWidget *parent)
  : QtHideAndShowDialog(parent),
    ui(new Ui::QtKalmanFilterExperimentDialog),
    m_examples_dialog{new QtKalmanFilterExamplesDialog},
    m_filter_dialog{new QtKalmanFilterDialog(model)},
    m_model{model},
    m_noise_parameters_dialog{new QtWhiteNoiseSystemParametersDialog(model)},
    m_parameters{}
{
  ui->setupUi(this);
  assert(m_examples_dialog);
  assert(m_filter_dialog);
  assert(m_model);
  assert(m_noise_parameters_dialog);
  assert(this->layout());

  //layout()->addWidget(new QLabel("Example settings"));
  layout()->addWidget(m_examples_dialog);

  //Create the map
  {
    const std::vector<KalmanFilterExperimentParameterType> v =
       KalmanFilterExperimentParameter().GetAll();
    const std::size_t sz = v.size();
    for (std::size_t i=0; i!=sz; ++i)
    {

      assert(i < v.size());
      const KalmanFilterExperimentParameterType type = v[i];

      //Skip the KalmanFilterParameterType instances (these are done by KalmanFilterDialog
      if (KalmanFilterExperimentParameter().CanConvertToKalmanFilterParameter(type))
      {
        continue;
      }
      if (KalmanFilterExperimentParameter().CanConvertToWhiteNoiseSystemParameter(type))
      {
        continue;
      }

      QtParameterDialog * const dialog
        = new QtParameterDialog(
          KalmanFilterExperimentParameter().ToName(type),
          KalmanFilterExperimentParameter().ToDescription(type),
          m_model->Find(type)
        );
      assert(dialog);

      //Add to map
      m_parameters.insert(std::make_pair(type,dialog));

      //Add to dialog
      assert(this->layout());
      this->layout()->addWidget(dialog);

    }
    assert(boost::numeric_cast<int>(v.size())
      == static_cast<int>(KalmanFilterExperimentParameterType::n_parameters
      ) && "All parameters must be in"
    );
  }

  layout()->addWidget(new QLabel("Kalman filter parameters"));
  layout()->addWidget(m_filter_dialog);
  layout()->addWidget(new QLabel("White noise system parameters"));
  layout()->addWidget(m_noise_parameters_dialog);

  assert(m_noise_parameters_dialog);
  assert(m_examples_dialog);

  //Connect clicking on the example buttons to setting (and showing) these
  m_examples_dialog->m_signal_example.connect(
    boost::bind(
      &ribi::kalman::QtKalmanFilterExperimentDialog::SetExample,this,boost::lambda::_1
    )
  );
  //When the model changes its number of timesteps, also display this new number
  m_model->m_signal_number_of_timesteps_changed.connect(
    boost::bind(
      &ribi::kalman::QtKalmanFilterExperimentDialog::SetNumberOfTimesteps,
      this,
      boost::lambda::_1
    )
  );
  //Make the white noise system parameters follow the possible tab changes in parameters
  this->GetFilterDialog()->m_signal_kalman_filter_type_changed.connect(
    boost::bind(
      &ribi::kalman::QtKalmanFilterExperimentDialog::SetKalmanFilterType,
      this,
      boost::lambda::_1
    )
  );
  ui->box_n_timesteps->setValue(5);

  assert(IsValid());
}

ribi::kalman::QtKalmanFilterExperimentDialog::~QtKalmanFilterExperimentDialog() noexcept
{
  delete ui;
  delete m_examples_dialog;
  delete m_filter_dialog;
  delete m_noise_parameters_dialog;
}

void ribi::kalman::QtKalmanFilterExperimentDialog::ClickExample(const int i)
{
  this->m_examples_dialog->EmitExample(i);
  assert(IsValid());
}

const ribi::kalman::QtKalmanFilterExamplesDialog *
  ribi::kalman::QtKalmanFilterExperimentDialog::GetExamplesDialog() const
{
  assert(m_examples_dialog);
  return m_examples_dialog;
}

ribi::kalman::QtKalmanFilterExamplesDialog *
  ribi::kalman::QtKalmanFilterExperimentDialog::GetExamplesDialog()
{
  assert(m_examples_dialog);
  return m_examples_dialog;
}

int ribi::kalman::QtKalmanFilterExperimentDialog::GetNumberOfTimesteps() const
{
  return ui->box_n_timesteps->value();
}

#ifndef NDEBUG
bool ribi::kalman::QtKalmanFilterExperimentDialog::IsValid() const
{
  if (this->GetNumberOfTimesteps() != m_model->GetNumberOfTimesteps())
  {
    return false;
  }
  if (GetNoiseParametersDialog()->GetWhiteNoiseSystemType()
    != m_model->CreateWhiteNoiseSystemParameters()->GetType())
  {
    return false;
  }
  if (GetFilterDialog()->GetKalmanFilterType()
    != m_model->CreateKalmanFilter()->GetType())
  {
    return false;
  }
  return true;
}
#endif

void ribi::kalman::QtKalmanFilterExperimentDialog::keyPressEvent(QKeyEvent * event)
{
  if (event->key() == Qt::Key_Escape) return;
  QDialog::keyPressEvent(event);
}


void ribi::kalman::QtKalmanFilterExperimentDialog::LoadFromDokuWiki(
  const std::string& filename
)
{
  assert(QFile::exists(filename.c_str()));
  std::string text;
  std::ifstream in(filename.c_str());
  std::string s;
  for (int i=0; !in.eof(); ++i)
  {
    std::getline(in,s);
    text += s + "\n";
  }
  this->m_model->FromDokuWiki(text);
  this->SetNumberOfTimesteps(m_model->GetNumberOfTimesteps());
  this->GetExamplesDialog()->SetShowNoExample();
  assert(IsValid());
}

void ribi::kalman::QtKalmanFilterExperimentDialog::on_box_n_timesteps_valueChanged(int arg1)
{
  m_model->SetNumberOfTimesteps(arg1);
  assert(m_model->GetNumberOfTimesteps() == this->GetNumberOfTimesteps());
  //Don't check here: this value might be set in a sequence of
  // SetNumberOfTimesteps - SetKalmanFilterType - SetWhiteNoiseSystemType
  //Everywhere in between this sequence, this dialog is in an invalid state
  //assert(IsValid());
}


void ribi::kalman::QtKalmanFilterExperimentDialog::on_button_add_state_clicked()
{
  //Get the state names
  QAbstractTableModel * const abstract_model
    = m_model->Find(KalmanFilterExperimentParameterType::state_names);
  assert(abstract_model);
  StateNamesModel * const model = dynamic_cast<StateNamesModel*>(abstract_model);
  assert(model);
  std::vector<std::string> v = model->GetRawData();
  v.push_back("?");
  model->SetRawData(v); //Set the data
  assert(model->GetRawData() == v);
  assert(IsValid());
}

void ribi::kalman::QtKalmanFilterExperimentDialog::on_button_remove_state_clicked()
{
  //Get the state names
  QAbstractTableModel * const abstract_model
    = m_model->Find(KalmanFilterExperimentParameterType::state_names);
  assert(abstract_model);
  StateNamesModel * const model = dynamic_cast<StateNamesModel*>(abstract_model);
  assert(model);
  std::vector<std::string> v = model->GetRawData();
  if (v.empty()) return;

  v.pop_back();
  model->SetRawData(v); //Set the data
  assert(model->GetRawData() == v);
  assert(IsValid());
}

void ribi::kalman::QtKalmanFilterExperimentDialog::on_button_load_clicked()
{
  if (ui->box_save->currentIndex() == 1)
  {
    QMessageBox b;
    b.setText("Loading from HTML not implemented");
    b.exec();
    return;
  }

  std::string filter;
  switch (ui->box_save->currentIndex())
  {
    case 0: //DokuWiki
      filter = "*.txt";
    break;
    case 1: //HTML
      filter = "*.htm";
    break;
    default:
      assert(!"Should not get here");
  }

  const QString filename = QFileDialog::getOpenFileName(
    0,
    "Please select a file to load the parameters from",
    "",
    filter.c_str());
  if (!filename.isEmpty())
  {
    switch (ui->box_save->currentIndex())
    {
      case 0: //DokuWiki
        this->LoadFromDokuWiki(filename.toStdString());
      break;
      case 1: //HTML
      {
        assert(!"Not implemented");
      }
      break;
      default:
        assert(!"Should not get here");
    }
  }

  assert(IsValid());
}

void ribi::kalman::QtKalmanFilterExperimentDialog::SaveToDokuWiki(const std::string& filename) const
{
  const std::string text = this->m_model->ToDokuWiki();
  std::ofstream f(filename.c_str());
  f << text;
}


void ribi::kalman::QtKalmanFilterExperimentDialog::on_button_save_clicked()
{
  std::string filter;
  switch (ui->box_save->currentIndex())
  {
    case 0: //DokuWiki
      filter = "*.txt";
    break;
    case 1: //HTML
      filter = "*.htm";
    break;
    default:
      assert(!"Should not get here");
  }

  const QString filename = QFileDialog::getSaveFileName(
    0,
    "Please select a file to save the parameters to",
    "",
    filter.c_str());
  if (!filename.isEmpty())
  {

    switch (ui->box_save->currentIndex())
    {
      case 0: //DokuWiki
      {
        SaveToDokuWiki(filename.toStdString());
      }
      break;
      case 1: //HTML
      {
        std::string text = this->m_model->ToHtml();
        std::ofstream f(filename.toStdString().c_str());
        f << text;
      }
      break;
      default:
        assert(!"Should not get here");
    }
  }
}

void ribi::kalman::QtKalmanFilterExperimentDialog::SetExample(
  const KalmanFilterExample * const example_raw
)
{
  assert(example_raw);
  const boost::shared_ptr<const KalmanFilterExample> example(example_raw);
  assert(example);
  assert(m_model);

  m_model->SetExample(example);

  #ifndef NDEBUG
  const boost::shared_ptr<KalmanFilterExperiment> experiment = m_model->CreateExperiment();
  assert(experiment);
  assert(experiment->GetKalmanFilter());
  assert(experiment->GetStateNames() == example->GetStateNames());
  assert(experiment->GetKalmanFilter()->GetParameters()->GetType()
    == example->GetKalmanFilterParameters()->GetType()
  );
  if (const boost::shared_ptr<const StandardKalmanFilterParameters> p_experiment
     = boost::dynamic_pointer_cast<
       const StandardKalmanFilterParameters
     >(experiment->GetKalmanFilter()->GetParameters())
  )
  {
    assert(p_experiment);
    const boost::shared_ptr<const StandardKalmanFilterParameters> p_example
      = boost::dynamic_pointer_cast<const StandardKalmanFilterParameters>(
        example->GetKalmanFilterParameters()
      );
    assert(p_example);
    assert(Matrix::MatricesAreEqual( p_experiment->GetControl(), p_example->GetControl() ) );
    assert(StandardKalmanFilterParameters::IsAboutEqual(*p_experiment,*p_example));
  }
  #endif
  assert(IsValid());
}

void ribi::kalman::QtKalmanFilterExperimentDialog::SetKalmanFilterType(const KalmanFilterType new_type)
{
  if (m_filter_dialog->GetKalmanFilterType() != new_type)
  {
    m_filter_dialog->SetKalmanFilterType(new_type);
    assert(m_filter_dialog->GetKalmanFilterType() == new_type);
  }
  assert(m_filter_dialog->GetKalmanFilterType() == new_type);
  assert(m_model->CreateKalmanFilterParameters()->GetType() == new_type);

  //Don't check here: this value might be set in a sequence of
  // SetNumberOfTimesteps - SetKalmanFilterType - SetWhiteNoiseSystemType
  //Everywhere in between this sequence, this dialog is in an invalid state
  //assert(IsValid());
}

void ribi::kalman::QtKalmanFilterExperimentDialog::SetNumberOfTimesteps(const int timesteps)
{
  assert(timesteps > 0);
  ui->box_n_timesteps->setValue(timesteps);
  assert(GetNumberOfTimesteps() == timesteps);
  assert(m_model->GetNumberOfTimesteps() == timesteps);

  //Don't check here: this value might be set in a sequence of
  // SetNumberOfTimesteps - SetKalmanFilterType - SetWhiteNoiseSystemType
  //Everywhere in between this sequence, this dialog is in an invalid state
  //assert(IsValid());
}
