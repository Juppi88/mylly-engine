MU_TEST(test_object_identity)
{
	mu_check(vec3_equals(obj_get_position(object), vec3_zero));
	mu_check(quat_equals(obj_get_rotation(object), quat_identity));
	mu_check(vec3_equals(obj_get_scale(object), vec3_one));

	mu_check(vec3_equals(obj_get_forward_vector(object), vec3_forward));
	mu_check(vec3_equals(obj_get_right_vector(object), vec3_right));
	mu_check(vec3_equals(obj_get_up_vector(object), vec3_up));
}

MU_TEST(test_object_directions)
{
	vec3_t euler = vec3(DEG_TO_RAD(0), DEG_TO_RAD(0), DEG_TO_RAD(90));
	quat_t rotation = quat_from_euler3(&euler);
	obj_set_local_rotation(object, rotation);

	//quat_print2(rotation);
	//quat_print2(obj_get_rotation(object));

	printf("\n");
	printf("RGT: "); vec3_print2(obj_get_right_vector(object));
	printf("UP : "); vec3_print2(obj_get_up_vector(object));
	printf("FWD: "); vec3_print2(obj_get_forward_vector(object));


	mu_check(quat_equals(obj_get_rotation(object), rotation));
}

void run_object(void)
{
	MU_RUN_TEST(test_object_identity);
	MU_RUN_TEST(test_object_directions);
}
