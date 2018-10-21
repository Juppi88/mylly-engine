#include "main.h"
#include "scene/scene.h"
#include "scene/object.h"
#include <stdio.h>

scene_t *scene;
object_t *parent;
object_t *object;

#include "quaternion.c"
#include "object.c"

static void test_setup(void)
{
	scene = scene_create();
	parent = scene_create_object(scene, NULL);
	object = scene_create_object(scene, parent);
}

static void test_teardown(void)
{
	scene_destroy(scene);
}

MU_TEST_SUITE(test_suite)
{
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

	run_quaternion();
	run_object();
}	

int main(void)
{
	printf("Running unit tests for Mylly...\n");

	MU_RUN_SUITE(test_suite);
	MU_REPORT();

	return 0;
}
