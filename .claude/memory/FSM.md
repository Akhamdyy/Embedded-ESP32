# Autonomous Wall-Following Robot FSM

## Purpose

This document describes the finite state machine (FSM) used by the autonomous wall-following robot. It is written in an LLM-friendly format so an AI assistant can understand the robot behavior, decision flow, and expected transitions.

---

# High-Level Behavior

The robot:

* Starts in an idle state.
* Searches for and follows a wall.
* Maintains a target distance from the wall.
* Re-centers itself if it drifts too close or too far.
* Turns left or right when obstacles or corners are detected.
* Stops if recovery fails or an unsafe condition occurs.

The control logic is event-driven and sensor-based.

---

# Sensors

The robot may use:

* Front distance sensor
* Left distance sensor
* Right distance sensor
* IMU / gyro (optional)
* Wheel encoders (optional)

Sensor values are interpreted using thresholds.

Example thresholds:

```txt
WALL_DETECTED_DISTANCE
TOO_CLOSE_DISTANCE
TOO_FAR_DISTANCE
FRONT_OBSTACLE_DISTANCE
CENTER_TOLERANCE
```

---

# FSM States

## 1. IDLE

### Description

Robot is inactive and waiting for a start command.

### Entry Actions

* Stop motors
* Reset temporary flags
* Initialize sensor readings

### Exit Condition

Transition to `WALL_FOLLOW` when:

```txt
start_command == true
```

---

## 2. WALL_FOLLOW

### Description

Primary operating state.
Robot moves forward while maintaining a stable distance from the wall.

### Behavior

* Move forward continuously
* Read side distance sensor
* Apply correction to maintain target wall distance
* Prefer smooth steering instead of abrupt turns

### Transition Conditions

#### To RECENTER

If robot drifts from desired wall distance:

```txt
wall_distance < TOO_CLOSE_DISTANCE
OR
wall_distance > TOO_FAR_DISTANCE
```

#### To TURN_LEFT

If front obstacle detected and left path is available:

```txt
front_distance < FRONT_OBSTACLE_DISTANCE
AND
left_path_open == true
```

#### To TURN_RIGHT

If front obstacle detected and right path is available:

```txt
front_distance < FRONT_OBSTACLE_DISTANCE
AND
right_path_open == true
```

#### To WALL_LOST

If no wall is detected:

```txt
wall_detected == false
```

#### To STOP

If emergency or unrecoverable condition occurs:

```txt
critical_error == true
```

---

## 3. WALL_LOST

### Description

Robot can no longer detect the wall.
It attempts to recover and reacquire wall alignment.

### Behavior

* Slow down
* Rotate slightly toward expected wall side
* Scan using side sensors

### Transition Conditions

#### To WALL_FOLLOW

If wall becomes visible again:

```txt
wall_detected == true
```

#### To RECENTER

If wall is detected but alignment is poor:

```txt
wall_detected == true
AND
alignment_error > CENTER_TOLERANCE
```

#### To STOP

If timeout expires:

```txt
wall_search_timeout == true
```

---

## 4. RECENTER

### Description

Robot corrects its position relative to the wall.
Used when the robot becomes too close or too far from the wall.

### Behavior

* Apply steering correction
* Reduce speed for stability
* Continue monitoring wall distance

### Transition Conditions

#### To WALL_FOLLOW

When centered:

```txt
abs(wall_distance - target_distance) <= CENTER_TOLERANCE
```

#### To TURN_LEFT

If correction requires strong left adjustment:

```txt
correction_direction == LEFT
```

#### To TURN_RIGHT

If correction requires strong right adjustment:

```txt
correction_direction == RIGHT
```

#### To WALL_LOST

If wall disappears during correction:

```txt
wall_detected == false
```

---

## 5. TURN_LEFT

### Description

Robot performs a controlled left turn.
Usually triggered by obstacles, corners, or recovery behavior.

### Behavior

* Left motor slower or reversed
* Right motor faster
* Monitor heading and sensor feedback

### Transition Conditions

#### To WALL_FOLLOW

When turn is complete and wall alignment restored:

```txt
turn_complete == true
AND
wall_detected == true
```

#### To RECENTER

If additional alignment correction is needed:

```txt
alignment_error > CENTER_TOLERANCE
```

---

## 6. TURN_RIGHT

### Description

Robot performs a controlled right turn.
Usually triggered by obstacles, corners, or recovery behavior.

### Behavior

* Right motor slower or reversed
* Left motor faster
* Monitor heading and sensor feedback

### Transition Conditions

#### To WALL_FOLLOW

When turn is complete and wall alignment restored:

```txt
turn_complete == true
AND
wall_detected == true
```

#### To RECENTER

If additional alignment correction is needed:

```txt
alignment_error > CENTER_TOLERANCE
```

---

## 7. STOP

### Description

Emergency or terminal state.
Robot halts all movement.

### Entry Actions

* Stop all motors
* Disable movement controller
* Raise error/debug flag

### Exit Condition

```txt
manual_reset == true
```

Transition back to:

```txt
IDLE
```

---

# Recommended LLM Interpretation Rules

When reasoning about this FSM:

1. `WALL_FOLLOW` is the default active navigation state.
2. `RECENTER` is a corrective state, not a navigation state.
3. `TURN_LEFT` and `TURN_RIGHT` are temporary maneuver states.
4. `WALL_LOST` is a recovery/search state.
5. `STOP` overrides all other states.
6. Safety conditions always have highest priority.
7. Sensor noise should be filtered before triggering transitions.
8. Prefer hysteresis to avoid rapid state oscillation.

---

# Suggested Internal Variables

```txt
current_state
previous_state
wall_distance
front_distance
left_distance
right_distance
target_distance
alignment_error
turn_complete
wall_detected
critical_error
```

---

# Suggested Transition Priority

Highest priority first:

```txt
1. STOP
2. WALL_LOST
3. TURN_LEFT / TURN_RIGHT
4. RECENTER
5. WALL_FOLLOW
```

---

# Example FSM Loop (Pseudo Logic)

```txt
while robot_running:

    read_sensors()

    switch(current_state):

        case IDLE:
            wait_for_start()

        case WALL_FOLLOW:
            follow_wall()

        case RECENTER:
            correct_alignment()

        case TURN_LEFT:
            execute_left_turn()

        case TURN_RIGHT:
            execute_right_turn()

        case WALL_LOST:
            search_for_wall()

        case STOP:
            stop_all_motion()
```

---

# Design Goal

The FSM is designed for:

* Stable wall following
* Robust recovery behavior
* Smooth navigation
* Predictable state transitions
* Easy portability to embedded systems (ESP32, STM32, ROS, etc.)
