#include <cogl/cogl.h>

#include <string.h>

#include "test-utils.h"

typedef struct _TestState
{
  int stub;
} TestState;

static void
paint (TestState *state)
{
  CoglPipeline *pipeline;
  CoglSnippet *snippet;
  CoglColor color;
  int location;
  int i;

  cogl_color_init_from_4ub (&color, 0, 0, 0, 255);
  cogl_clear (&color, COGL_BUFFER_BIT_COLOR);

  /* Simple fragment snippet */
  pipeline = cogl_pipeline_new ();

  cogl_pipeline_set_color4ub (pipeline, 255, 0, 0, 255);

  snippet = cogl_snippet_new (NULL, "cogl_color_out.g += 1.0;");
  cogl_pipeline_add_fragment_hook (pipeline, snippet);
  cogl_object_unref (snippet);

  cogl_push_source (pipeline);
  cogl_rectangle (0, 0, 10, 10);
  cogl_pop_source ();

  cogl_object_unref (pipeline);

  /* Simple vertex snippet */
  pipeline = cogl_pipeline_new ();

  cogl_pipeline_set_color4ub (pipeline, 255, 0, 0, 255);

  snippet = cogl_snippet_new (NULL, "cogl_color_out.b += 1.0;");
  cogl_pipeline_add_vertex_hook (pipeline, snippet);
  cogl_object_unref (snippet);

  cogl_push_source (pipeline);
  cogl_rectangle (10, 0, 20, 10);
  cogl_pop_source ();

  cogl_object_unref (pipeline);

  /* Single snippet used with in both the vertex and fragment hooks
     with a uniform */
  pipeline = cogl_pipeline_new ();

  location = cogl_pipeline_get_uniform_location (pipeline, "a_value");
  cogl_pipeline_set_uniform_1f (pipeline, location, 0.25f);

  cogl_pipeline_set_color4ub (pipeline, 255, 0, 0, 255);

  snippet = cogl_snippet_new ("uniform float a_value;",
                              "cogl_color_out.b += a_value;");
  cogl_pipeline_add_fragment_hook (pipeline, snippet);
  cogl_pipeline_add_vertex_hook (pipeline, snippet);
  cogl_object_unref (snippet);

  cogl_push_source (pipeline);
  cogl_rectangle (20, 0, 30, 10);
  cogl_pop_source ();

  cogl_object_unref (pipeline);

  /* Lots of snippets on one pipeline */
  pipeline = cogl_pipeline_new ();

  cogl_pipeline_set_color4ub (pipeline, 0, 0, 0, 255);

  for (i = 0; i < 3; i++)
    {
      char letter = 'x' + i;
      char *uniform_name = g_strdup_printf ("%c_value", letter);
      char *declarations = g_strdup_printf ("uniform float %s;\n",
                                            uniform_name);
      char *code = g_strdup_printf ("cogl_color_out.%c = %s;\n",
                                    letter,
                                    uniform_name);

      location = cogl_pipeline_get_uniform_location (pipeline, uniform_name);
      cogl_pipeline_set_uniform_1f (pipeline, location, (i + 1) * 0.1f);

      snippet = cogl_snippet_new (declarations, code);
      cogl_pipeline_add_fragment_hook (pipeline, snippet);
      cogl_object_unref (snippet);

      g_free (code);
      g_free (uniform_name);
      g_free (declarations);
    }

  cogl_push_source (pipeline);
  cogl_rectangle (30, 0, 40, 10);
  cogl_pop_source ();

  cogl_object_unref (pipeline);

  /* The pre string can't really do anything with the current hooks,
     but let's just test that it compiles */
  pipeline = cogl_pipeline_new ();

  cogl_pipeline_set_color4ub (pipeline, 255, 0, 0, 255);

  snippet = cogl_snippet_new (NULL, NULL);
  cogl_snippet_set_pre (snippet, "cogl_color_out = vec4 (1.0, 0.5, 0.8, 1.0);");
  cogl_pipeline_add_vertex_hook (pipeline, snippet);
  cogl_pipeline_add_fragment_hook (pipeline, snippet);
  cogl_object_unref (snippet);

  cogl_push_source (pipeline);
  cogl_rectangle (40, 0, 50, 10);
  cogl_pop_source ();

  cogl_object_unref (pipeline);

  /* Check that the pipeline caching works when unrelated pipelines
     share snippets state. It's too hard to actually assert this in
     the conformance test but at least it should be possible to see by
     setting COGL_DEBUG=show-source to check whether this shader gets
     generated twice */
  snippet = cogl_snippet_new ("/* This comment should only be seen ONCE\n"
                              "   when COGL_DEBUG=show-source is TRUE\n"
                              "   even though it is used in two different\n"
                              "   unrelated pipelines */",
                              "cogl_color_out = vec4 (0.0, 1.0, 0.0, 1.0);\n");

  pipeline = cogl_pipeline_new ();
  cogl_pipeline_add_fragment_hook (pipeline, snippet);
  cogl_push_source (pipeline);
  cogl_rectangle (50, 0, 60, 10);
  cogl_pop_source ();
  cogl_object_unref (pipeline);

  pipeline = cogl_pipeline_new ();
  cogl_pipeline_add_fragment_hook (pipeline, snippet);
  cogl_push_source (pipeline);
  cogl_rectangle (60, 0, 70, 10);
  cogl_pop_source ();
  cogl_object_unref (pipeline);

  cogl_object_unref (snippet);

  /* Sanity check modifying the snippet */
  snippet = cogl_snippet_new ("foo", "bar");
  g_assert_cmpstr (cogl_snippet_get_declarations (snippet), ==, "foo");
  g_assert_cmpstr (cogl_snippet_get_post (snippet), ==, "bar");
  g_assert_cmpstr (cogl_snippet_get_pre (snippet), ==, NULL);

  cogl_snippet_set_declarations (snippet, "fu");
  g_assert_cmpstr (cogl_snippet_get_declarations (snippet), ==, "fu");
  g_assert_cmpstr (cogl_snippet_get_post (snippet), ==, "bar");
  g_assert_cmpstr (cogl_snippet_get_pre (snippet), ==, NULL);

  cogl_snippet_set_post (snippet, "ba");
  g_assert_cmpstr (cogl_snippet_get_declarations (snippet), ==, "fu");
  g_assert_cmpstr (cogl_snippet_get_post (snippet), ==, "ba");
  g_assert_cmpstr (cogl_snippet_get_pre (snippet), ==, NULL);

  cogl_snippet_set_pre (snippet, "fuba");
  g_assert_cmpstr (cogl_snippet_get_declarations (snippet), ==, "fu");
  g_assert_cmpstr (cogl_snippet_get_post (snippet), ==, "ba");
  g_assert_cmpstr (cogl_snippet_get_pre (snippet), ==, "fuba");
}

static void
validate_result (void)
{
  test_utils_check_pixel (5, 5, 0xffff00ff);
  test_utils_check_pixel (15, 5, 0xff00ffff);
  test_utils_check_pixel (25, 5, 0xff0080ff);
  test_utils_check_pixel (35, 5, 0x19334cff);
  test_utils_check_pixel (45, 5, 0xff0000ff);
  test_utils_check_pixel (55, 5, 0x00ff00ff);
  test_utils_check_pixel (65, 5, 0x00ff00ff);
}

void
test_cogl_snippets (TestUtilsGTestFixture *fixture,
                    void *user_data)
{
  TestUtilsSharedState *shared_state = user_data;

  /* If shaders aren't supported then we can't run the test */
  if (cogl_features_available (COGL_FEATURE_SHADERS_GLSL))
    {
      TestState state;

      cogl_ortho (/* left, right */
                  0, cogl_framebuffer_get_width (shared_state->fb),
                  /* bottom, top */
                  cogl_framebuffer_get_height (shared_state->fb), 0,
                  /* z near, far */
                  -1, 100);

      paint (&state);
      validate_result ();

      if (g_test_verbose ())
        g_print ("OK\n");
    }
  else if (g_test_verbose ())
    g_print ("Skipping\n");
}
