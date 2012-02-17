<?php

/**
 * @file
 * Forms for creating, editing, and deleting organizations.
 */

/**
 * Return a form for adding an organization.
 *
 * @param string $type
 *   The type of organization to return a form for.
 */
function redhen_org_add_page($type = 'default') {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);

  // Bypass the redhen_org/add listing if only one contact type is available.
  if (count($content) == 1) {
    $item = array_shift($content);
    drupal_goto($item['href']);
  }
  return theme('redhen_org_add_list', array('content' => $content));
}

function redhen_org_add($type) {
  $org = entity_get_controller('redhen_org')->create(array('type' => $type));
  return drupal_get_form('redhen_org_org_form', $org);
}

/**
 * Form callback: create or edit a contact.
 *
 * @param $org
 *   The organization object to edit or for a create form an empty organization object
 *     with only an org type defined.
 */
function redhen_org_org_form($form, &$form_state, $org) {
  // Ensure this include file is loaded when the form is rebuilt from the cache.
  $form_state['build_info']['files']['form'] = drupal_get_path('module', 'redhen_org') . '/includes/redhen_org.forms.inc';

  // Add the default field elements.
  $form['name'] = array();
  $form['name']['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => $org->label,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -6,
  );
  $form['status'] = array(
    '#type' => 'select',
    '#title' => t('Status'),
    '#description' => t('Organization status.'),
    '#options' => array(1 => t('Active'), 0 => t('Archived')), //@TODO: these should use the defined constants
    '#default_value' => $org->status,
    '#weight' => 10,
  );


  // Add the field related form elements.
  $form_state['redhen_org'] = $org;
  field_attach_form('redhen_org', $org, $form, $form_state);

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 40,
  );

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = array();

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save organization'),
    '#submit' => $submit + array('redhen_org_org_form_submit'),
  );

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'redhen_org_org_form_validate';

  return $form;
}

/**
 * Validation callback for redhen_org_org_form().
 */
function redhen_org_org_form_validate($form, &$form_state) {
  $org = $form_state['redhen_org'];

  // Notify field widgets to validate their data.
  field_attach_form_validate('redhen_org', $org, $form, $form_state);
}

/**
 * Submit callback for redhen_org_org_form().
 */
function redhen_org_org_form_submit($form, &$form_state) {
  $org = &$form_state['redhen_org'];

  // Set the contact's author uid
  global $user;
  $org->author_uid = $user->uid;

  // Save default parameters back into the $org object.
  $org->label = $form_state['values']['label'];
  $org->status = $form_state['values']['status'];

  // @TODO Add a log field to existing entries
  $org->log = '';

  // Notify field widgets.
  field_attach_submit('redhen_org', $org, $form, $form_state);

  // Save the contact.
  $org = redhen_org_save($org);

  drupal_set_message(t('Organization saved.'));
  $form_state['redirect'] = 'redhen/org/' . $org->org_id;
}

/**
 * Form callback: confirmation form for deleting an organization.
 *
 * @param $org
 *   The organization object to be deleted.
 *
 * @see confirm_form()
 */
function redhen_org_org_delete_form($form, &$form_state, $org) {
  $form_state['redhen_org'] = $org;

  // Ensure this include file is loaded when the form is rebuilt from the cache.
  $form_state['build_info']['files']['form'] = drupal_get_path('module', 'redhen_org') . '/includes/redhen_org.forms.inc';

  $form['#submit'][] = 'redhen_org_org_delete_form_submit';

  $title = $org->label;

  $form = confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $title)),
    '',
    '<p>' . t('Deleting this organization cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Submit callback for redhen_org_org_delete_form().
 */
function redhen_org_org_delete_form_submit($form, &$form_state) {
  $org = $form_state['redhen_org'];

  $title = $org->label;

  if (redhen_org_delete_multiple(array($org->org_id))) {
    drupal_set_message(t('%title has been deleted.', array('%title' => $title)));
    watchdog('redhen_org', 'Deleted organization %title.', array('%title' => $title), WATCHDOG_NOTICE);
  }
  else {
    drupal_set_message(t('%title could not be deleted.', array('%title' => $title)), 'error');
  }
}

/**
 * Returns HTML for a list of available org types for org creation.
 *
 * @param $variables
 *   An associative array containing:
 *   - content: An array of content types.
 *
 * @ingroup themeable
 */
function theme_redhen_org_add_list($variables) {
  $content = $variables['content'];
  $output = '';

  if ($content) {
    $output = '<dl class="node-type-list">';
    foreach ($content as $item) {
      $output .= '<dt>' . l($item['title'], $item['href'], $item['localized_options']) . '</dt>';
      $output .= '<dd>' . filter_xss_admin($item['description']) . '</dd>';
    }
    $output .= '</dl>';
  }
  else {
    $output = '<p>' . t('You have not created any organization types yet. Go to the <a href="@create-org-types">organization type creation page</a> to add a new content type.', array('@create-org-types' => url('admin/structure/redhen/org-types'))) . '</p>';
  }
  return $output;
}