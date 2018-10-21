MU_TEST(test_quat_from_euler)
{
	vec3_t euler;
	quat_t result;

	euler = vector3(DEG_TO_RAD(0), DEG_TO_RAD(0), DEG_TO_RAD(0));
	result = quaternion(0, 0, 0, 1);
	mu_check(quat_equals(quat_from_euler3(&euler), result));

	euler = vector3(DEG_TO_RAD(32), DEG_TO_RAD(89), DEG_TO_RAD(152));
	result = quaternion(-0.618516f, -0.701305f, 0.027762f, 0.353325f);
	mu_check(quat_equals(quat_from_euler3(&euler), result));

}

MU_TEST(test_quat_to_euler)
{
	vec3_t result;
	quat_t quat;

	quat = quaternion(0, 0, 0, 1);
	result = vector3(DEG_TO_RAD(0), DEG_TO_RAD(0), DEG_TO_RAD(0));
	mu_check(vec3_equals(quat_to_euler(&quat), result));

	quat = quaternion(-0.618516f, -0.701305f, 0.027762f, 0.353325f);
	result = vector3(DEG_TO_RAD(32), DEG_TO_RAD(89), DEG_TO_RAD(152));
	mu_check(vec3_equals(quat_to_euler(&quat), result));

	quat = quaternion(0.194612f, -0.362885f, -0.865079f, 0.286495f);
	result = vector3(DEG_TO_RAD(33), DEG_TO_RAD(155), DEG_TO_RAD(38));
	mu_check(vec3_equals(quat_to_euler(&quat), result));
}

MU_TEST(test_quat_to_from_euler)
{
	vec3_t euler;
	quat_t result;

	euler = vec3_zero;
	result = quat_identity;
	mu_check(vec3_equals(quat_to_euler(&result), euler));

	euler = vector3(DEG_TO_RAD(32), DEG_TO_RAD(89), DEG_TO_RAD(152));
	result = quat_from_euler3(&euler);
	mu_check(vec3_equals(quat_to_euler(&result), euler));

	euler = vector3(DEG_TO_RAD(33), DEG_TO_RAD(155), DEG_TO_RAD(38));
	result = quat_from_euler3(&euler);
	mu_check(vec3_equals(quat_to_euler(&result), euler));
}

void run_quaternion(void)
{
	MU_RUN_TEST(test_quat_from_euler);
	MU_RUN_TEST(test_quat_to_euler);
	MU_RUN_TEST(test_quat_to_from_euler);
}
