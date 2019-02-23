/*
 * Copyright (c) 2011, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

#include <QPainter>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

#include <QComboBox>
#include <QProgressBar>
#include <QDoubleSpinBox>
#include <QDial>

//#include "drive_widget.h"
#include "battery_panel.h"

namespace rviz_plugins
{

// BEGIN_TUTORIAL
// Here is the implementation of the TeleopPanel class.  TeleopPanel
// has these responsibilities:
//
// - Act as a container for GUI elements DriveWidget and QLineEdit.
// - Publish command velocities 10 times per second (whether 0 or not).
// - Saving and restoring internal state from a config file.
//
// We start with the constructor, doing the standard Qt thing of
// passing the optional *parent* argument on to the superclass
// constructor, and also zero-ing the velocities we will be
// publishing.
BatteryPanel::BatteryPanel( QWidget* parent )
  : rviz::Panel( parent )
//  , linear_velocity_( 0 )
//  , angular_velocity_( 0 )
{
  // Next we lay out the "output topic" text entry field using a
  // QLabel and a QLineEdit in a QHBoxLayout.
  QHBoxLayout *topic_layout = new QHBoxLayout;
  topic_layout->addWidget( new QLabel( "Input Topic:" ));
  input_topic_editor_ = new QLineEdit;
  input_topic_editor_->setFixedWidth(230);
  topic_layout->addWidget( input_topic_editor_ );

  // charging state indicator:
  QHBoxLayout *charge_state_layout = new QHBoxLayout;
  charge_state_layout->addWidget( new QLabel("Charger State:"));
  charge_state_combo_ = new QComboBox;
  charge_state_combo_->setEditable(false);
  charge_state_combo_->addItem("(0) UNKNOWN");
  charge_state_combo_->addItem("(1) CHARGING");
  charge_state_combo_->addItem("(2) DISCHARGING");
  charge_state_combo_->addItem("(3) NOT CHARGING");
  charge_state_combo_->addItem("(4) FULL");
  charge_state_combo_->setFixedWidth(230);
  charge_state_layout->addWidget( charge_state_combo_ );

  // Voltage gauge:
  QVBoxLayout *voltage_gauge_layout = new QVBoxLayout;
  voltage_gauge_layout->addWidget(new QLabel("  --- Voltage ---"));
  voltage_value_ = new QDoubleSpinBox;
  voltage_value_->setReadOnly(true);
  voltage_value_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  voltage_value_->setValue(8.43);
  voltage_value_->setSuffix(" V");
  voltage_value_->setFixedWidth(100);
  voltage_value_->setAlignment(Qt::AlignHCenter);
  voltage_gauge_layout->addWidget(voltage_value_);
  voltage_gauge_ = new QDial;
  voltage_gauge_->setNotchesVisible(true);
  voltage_gauge_->setMinimum(60);
  voltage_gauge_->setMaximum(100);
  voltage_gauge_->setValue((int)(8.43 * 10));
  voltage_gauge_->setFixedSize(100, 100);
  voltage_gauge_layout->addWidget(voltage_gauge_);
  voltage_gauge_layout->addWidget(new QLabel("   6.0          10.0"));
  voltage_gauge_layout->addStretch();

  // Current gauge:
  QVBoxLayout *current_gauge_layout = new QVBoxLayout;
  current_gauge_layout->addWidget(new QLabel("  --- Current ---"));
  current_value_ = new QDoubleSpinBox;
  current_value_->setReadOnly(true);
  current_value_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  current_value_->setValue(2.1);
  current_value_->setMinimum(-2.0);
  current_value_->setSuffix(" A");
  current_value_->setFixedWidth(100);
  current_value_->setAlignment(Qt::AlignHCenter);
  current_gauge_layout->addWidget(current_value_);
  current_gauge_ = new QDial;
  current_gauge_->setNotchesVisible(true);
  current_gauge_->setMinimum(0);
  current_gauge_->setMaximum(40);
  current_gauge_->setValue((int)(2.1 * 10));
  current_gauge_->setFixedSize(100, 100);
  current_gauge_layout->addWidget(current_gauge_);
  current_gauge_layout->addWidget(new QLabel("   0.0           4.0"));
  current_gauge_layout->addStretch();

  // battery percentage gauge:
  QVBoxLayout* battery_gauge_layout = new QVBoxLayout;
  battery_gauge_layout->addWidget( new QLabel(" Charge"));
  battery_gauge_ = new QProgressBar;
  battery_gauge_->setOrientation(Qt::Vertical);
  battery_gauge_->setAlignment(Qt::AlignHCenter);
  battery_gauge_->setValue(75);
  battery_gauge_->setFixedSize(55,102);
  battery_gauge_layout->addWidget( battery_gauge_ );
  battery_gauge_layout->addWidget( new QLabel("Resistance"));
  battery_resistance_ = new QDoubleSpinBox;
  battery_resistance_->setReadOnly(true);
  battery_resistance_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  battery_resistance_->setValue(0.1);
  battery_resistance_->setDecimals(1);
  battery_resistance_->setSuffix(" mΩ");
  battery_resistance_->setFixedWidth(75);
  battery_resistance_->setAlignment(Qt::AlignHCenter);
  battery_gauge_layout->addWidget(battery_resistance_);
  battery_gauge_layout->addStretch();

  // lay out gauges horizontally:
  QHBoxLayout *gauges_layout = new QHBoxLayout;
  gauges_layout->addLayout(voltage_gauge_layout);
  gauges_layout->addLayout(current_gauge_layout);
  gauges_layout->addLayout(battery_gauge_layout);

  // Lay out the topic field above the control widget.
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addLayout( topic_layout );
  layout->addLayout( charge_state_layout );
  layout->addLayout( gauges_layout );

  setLayout( layout );

  // Create a timer for receiving battery status.
  // Here we take advantage of QObject's memory management behavior:
  // since "this" is passed to the new QTimer as its parent, the
  // QTimer is deleted by the QObject destructor when this TeleopPanel
  // object is destroyed.  Therefore we don't need to keep a pointer
  // to the timer.
  QTimer* input_timer = new QTimer( this );

  // Next we make signal/slot connections.
//  connect( drive_widget_, SIGNAL( outputVelocity( float, float )), this, SLOT( setVel( float, float )));
  connect( input_topic_editor_, SIGNAL( editingFinished() ), this, SLOT( updateTopic() ));
  connect( input_timer, SIGNAL( timeout() ), this, SLOT( updateBatteryState() ));

  // Start the timer with 100 msec interval
  input_timer->start( 100 );

  // Make the control widget start disabled, since we don't start with an output topic.
//  drive_widget_->setEnabled( false );
}

/*
// setVel() is connected to the DriveWidget's output, which is sent
// whenever it changes due to a mouse event.  This just records the
// values it is given.  The data doesn't actually get sent until the
// next timer callback.
void TeleopPanel::setVel( float lin, float ang )
{
  linear_velocity_ = lin;
  angular_velocity_ = ang;
}
*/

// Read the topic name from the QLineEdit and call setTopic() with the
// results.  This is connected to QLineEdit::editingFinished() which
// fires when the user presses Enter or Tab or otherwise moves focus
// away.
void BatteryPanel::updateTopic()
{
  setTopic( input_topic_editor_->text() );
}

// Set the topic name we are publishing to.
void BatteryPanel::setTopic( const QString& new_topic )
{
  // Only take action if the name has changed.
  if( new_topic != input_topic_ )
  {
    input_topic_ = new_topic;
    // If the topic is the empty string, stop subscribing.
    if( input_topic_ == "" )
    {
      battery_subscriber_.shutdown();
    }
    else
    {
      // The old ``battery_subscriber_`` is destroyed by this assignment,
      // and thus the old topic subscription is removed.  The call to
      // nh_subscribe() says we want to subscribe to the new topic name.
      battery_subscriber_ = nh_.subscribe("battery", 10, &BatteryPanel::callbackBatteryState, this);
    }
    // rviz::Panel defines the configChanged() signal.  Emitting it
    // tells RViz that something in this panel has changed that will
    // affect a saved config file.  Ultimately this signal can cause
    // QWidget::setWindowModified(true) to be called on the top-level
    // rviz::VisualizationFrame, which causes a little asterisk ("*")
    // to show in the window's title bar indicating unsaved changes.
    Q_EMIT configChanged();
  }

  // Gray out the control widget when the output topic is empty.
  setEnabled( input_topic_ != "" );
}

// Enable or Disable all controls
void BatteryPanel::setEnabled(bool enable)
{
  charge_state_combo_->setEnabled(enable);
  voltage_value_->setEnabled(enable);
  voltage_gauge_->setEnabled(enable);
  current_value_->setEnabled(enable);
  current_gauge_->setEnabled(enable);
  battery_gauge_->setEnabled(enable);
  battery_resistance_->setEnabled(enable);
}

// Callback for receiving BatteryState messages
void BatteryPanel::callbackBatteryState(const sensor_msgs::BatteryState& bat_state)
{
  battery_state_ = bat_state;
}


// Process the received battery state if ROS is not shutting down and the
// subscriber is ready with a valid topic name.
void BatteryPanel::updateBatteryState()
{
  if( ros::ok() && battery_subscriber_ )
  {
    charge_state_combo_->setCurrentIndex(battery_state_.power_supply_status);
    voltage_value_->setValue(battery_state_.voltage);
    voltage_gauge_->setValue((int)(battery_state_.voltage * 10));
    current_value_->setValue(battery_state_.current);
    current_gauge_->setValue((int)(fabs(battery_state_.current) * 10));
    battery_gauge_->setValue((int)(battery_state_.percentage * 100));
    battery_resistance_->setValue(battery_state_.charge);
  }
}


// Save all configuration data from this panel to the given
// Config object.  It is important here that you call save()
// on the parent class so the class id and panel name get saved.
void BatteryPanel::save( rviz::Config config ) const
{
  rviz::Panel::save( config );
  config.mapSetValue( "Topic", input_topic_ );
}

// Load all configuration data for this panel from the given Config object.
void BatteryPanel::load( const rviz::Config& config )
{
  rviz::Panel::load( config );
  QString topic;
  if( config.mapGetString( "Topic", &topic ))
  {
    input_topic_editor_->setText( topic );
    updateTopic();
  }
}

} // end namespace rviz_plugins



// Tell pluginlib about this class.  Every class which should be
// loadable by pluginlib::ClassLoader must have these two lines
// compiled in its .cpp file, outside of any namespace scope.
#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz_plugins::BatteryPanel,rviz::Panel )
// END_TUTORIAL
