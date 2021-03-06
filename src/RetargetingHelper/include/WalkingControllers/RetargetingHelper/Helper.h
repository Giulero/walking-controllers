/**
 * @file Helper.h
 * @authors Giulio Romualdi <giulio.romualdi@iit.it>
 * @copyright 2019 iCub Facility - Istituto Italiano di Tecnologia
 *            Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 * @date 2019
 */

#ifndef WALKING_CONTROLLERS_RETARGETING_HELPER_HELPER_H
#define WALKING_CONTROLLERS_RETARGETING_HELPER_HELPER_H

// std
#include <memory>
#include <vector>

// iDyntree
#include <iDynTree/Core/Transform.h>
#include <iDynTree/Core/Rotation.h>
#include <iDynTree/Core/VectorDynSize.h>

// yarp
#include <yarp/os/BufferedPort.h>

#include <yarp/sig/Vector.h>

// iCub-ctrl
#include <iCub/ctrl/minJerkCtrl.h>

#include <WalkingControllers/KinDynWrapper/Wrapper.h>

namespace WalkingControllers
{

/**
 * Client for the retargeting application
 */
    class RetargetingClient
    {
    public:
      enum class Phase
      {
          /** In this phase the smoothing time of the minimum jerk trajectory is
              increased. This will guarantee a smoother transition between the
              initial joint configuration and the desired joint configuration. */
          approacing,
          stance,
          walking
      };

    private:

        template <class Data>
        struct RetargetingElement
        {
            yarp::sig::Vector yarpReadBuffer;
            std::unique_ptr<iCub::ctrl::minJerkTrajGen> smoother;
            yarp::os::BufferedPort<yarp::sig::Vector> port;
            double smoothingTimeInApproaching;
            double smoothingTimeInWalking;
            Data data;
        };

        bool m_useHandRetargeting; /**< True if the hand retargeting is used */
        bool m_useVirtualizer; /**< True if the virtualizer is used */
        bool m_useJointRetargeting; /**< True if the joint retargeting is used */
        bool m_useCoMHeightRetargeting; /**< True if the com retargeting is used */

        RetargetingElement<iDynTree::Transform> m_leftHand; /**< Left hand retargeting element */
        RetargetingElement<iDynTree::Transform> m_rightHand; /**< Right hand retargeting element */

        /** Offset of the CoM Height coming from the user. It is required given the different size
         *  between the human and the robot */
        double m_comHeightInputOffset;

        /** Desired value of the CoM height used during walking. The simplified model used for
         * the locomotion is based on the assumption of a constant CoM height */
        double m_comConstantHeight;

        /** Factor required to scale the human CoM displacement to a desired robot CoM displacement */
        double m_comHeightScalingFactor;

        struct KinematicState
        {
            double position;
            double velocity;
        };
        RetargetingElement<KinematicState> m_comHeight;

        std::vector<int> m_retargetJointsIndex; /**< Vector containing the indices of the retargeted joints. */
        RetargetingElement<iDynTree::VectorDynSize> m_jointRetargeting; /**< Joint retargeting element */

        yarp::os::BufferedPort<yarp::sig::Vector> m_robotOrientationPort; /**< Average orientation of the robot.*/

        Phase m_phase{Phase::approacing};
        double m_startingApproachingPhaseTime; /**< Initial time of the approaching phase (seconds) */
        double m_approachPhaseDuration; /**< Duration of the approaching phase (seconds) */

        /**
         * Convert a yarp vector containing position + rpy into an iDynTree homogeneous transform
         * @param vector a 6d yarp vector
         * @param transform an iDyntree homogeneous transformation
         */
        void convertYarpVectorPoseIntoTransform(const yarp::sig::Vector& vector,
                                                iDynTree::Transform& transform);

        /**
         * Terminate the approaching phase
         */
        void stopApproachingPhase();

    public:

        /**
         * Initialize the client
         * @param config configuration parameters
         * @param name name of the module
         * @param period period of the module
         * @param controlledJointsName name of the controlled joints
         * @return true/false in case of success/failure
         */
        bool initialize(const yarp::os::Searchable &config,
                        const std::string &name,
                        const double &period,
                        const std::vector<std::string>& controlledJointNames);

        /**
         * Reset the client
         * @param kinDynWrapper a wrapper of KinDynComputations useful to get some
         * informations of the current status of the robot
         * @return true/false in case of success/failure
         */
        bool reset(WalkingFK& kinDynWrapper);

        /**
         * Close the client
         */
        void close();

        /**
         * Get the feedback of the server
         */
        void getFeedback();

        /**
         * Get the homogeneous transform of the left hand w.r.t. the head frame head_T_leftHand
         */
        const iDynTree::Transform& leftHandTransform() const;

        /**
         * Get the homogeneous transform of the right hand w.r.t. the head frame head_T_rightHand
         */
        const iDynTree::Transform& rightHandTransform() const;

        /**
         * Get the value of the retarget joints
         */
        const iDynTree::VectorDynSize& jointValues() const;

        /**
         * Get the CoM height
         * @return height of the CoM
         */
        double comHeight() const;

        /**
         * Get the CoM height velocity
         * @return height velocity of the CoM
         */
        double comHeightVelocity() const;

        /**
         * Get the homogeneous transform of the right hand w.r.t. the head frame head_T_rightHand
         */
        void setRobotBaseOrientation(const iDynTree::Rotation& rotation);


        void setPhase(Phase phase);

        /**
         * Start the approaching phase
         */
        void startApproachingPhase();

        /**
         * Check if the approaching phase is running
         * @return true if the approaching phase is running
         */
        bool isApproachingPhase() const;
    };
};
#endif
