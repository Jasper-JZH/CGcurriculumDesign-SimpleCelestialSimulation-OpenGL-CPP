#pragma once
# include<glad/glad.h>
# include<glm/glm.hpp>
# include<glm/gtc/matrix_transform.hpp>

#include<vector>

//定义相关相机位移方向
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

//默认相机参数 
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 45.0f;

class Camera
{
public:
	//定义相机参数
	glm::vec3 Position;
	glm::vec3 Front;	//前后移速
	glm::vec3 Up;
	glm::vec3 Right;	//左右移速
	glm::vec3 WorldUp;

	glm::vec3 fisrtPersonCameraOriPos = glm::vec3(0.0f, 2.0f, 10.0f);
	glm::vec3 thirdPersonCameraOriPos = glm::vec3(0.0f, 0.0f, 20.0f);

	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	bool isFirstPerson = true;

	float pitchLim = 89.0f;

	float distance;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		//算出相机到观察中心的距离
		distance = glm::distance(glm::vec3(0.0f, 0.0f, 0.0f), Position);
		updateCameraVectors();
	}

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		//算出相机到观察中心的距离
		distance = glm::distance(glm::vec3(0.0f, 0.0f, 0.0f), Position);
		updateCameraVectors();
	}

	void ChangeMode()
	{
		isFirstPerson = !isFirstPerson;
		if (isFirstPerson)
		{
			pitchLim = 89.0f;
			Yaw = -90.0f;
			Pitch = 0.0f;
			Position = fisrtPersonCameraOriPos;
		}
		else
		{
			pitchLim = 60.0f;
			Yaw = 0.0f;
			Pitch = 0.0f;
			Position = thirdPersonCameraOriPos;
		}
		Zoom = ZOOM;
		updateCameraVectors();
	}

	//获取V矩阵
	glm::mat4 GetViewMatrix()
	{
		if (isFirstPerson)
			return glm::lookAt(Position, Position + Front, Up);
		else
			return glm::lookAt(Position, glm::vec3(0.0f, 0.0f, 0.0f), Up);
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		if (isFirstPerson)
		{
			float velocity = MovementSpeed * deltaTime;
			if (direction == FORWARD)
				Position += Front * velocity;
			if (direction == BACKWARD)
				Position -= Front * velocity;
			if (direction == LEFT)
				Position -= Right * velocity;
			if (direction == RIGHT)
				Position += Right * velocity;
		}
	}

	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch = isFirstPerson ? Pitch + yoffset : Pitch - yoffset;

		if (constrainPitch)
		{
			if (Pitch > pitchLim)
				Pitch = pitchLim;
			if (Pitch < -pitchLim)
				Pitch = -pitchLim;
		}
		updateCameraVectors();
	}

	void ProcessMouseScroll(float scroll_offset)
	{
		Zoom -= (float)scroll_offset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

private:
	void updateCameraVectors()
	{
		if (isFirstPerson)
		{
			glm::vec3 front;
			front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			front.y = sin(glm::radians(Pitch));
			front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			Front = glm::normalize(front);
		}
		else
		{
			glm::vec3 newPos;
			newPos.x = distance * cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			newPos.y = distance * sin(glm::radians(Pitch));
			newPos.z = distance * sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			Position = newPos;
			Front = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - Position);
		}
		//根据前向向量计算出相机的其他向量
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}
};