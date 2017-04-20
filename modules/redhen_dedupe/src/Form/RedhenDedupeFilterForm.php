<?php

/**
 * @file
 * Contains \Drupal\redhen_dedupe\Form\RedhenDedupeFilterForm.
 */

namespace Drupal\redhen_dedupe\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Render\Element;
use \Drupal\redhen_contact\Entity\Contact;

class RedhenDedupeFilterForm extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'redhen_dedupe_filter_form';
  }

  public function buildForm(array $form, FormStateInterface $form_state, $base_fields = array(), $fields = array(), $active = TRUE) {
    $bundles = \Drupal::service('entity_type.bundle.info')->getBundleInfo('redhen_contact');
    $info = array();
    foreach (array_keys($bundles) as $bundle) {
      $info['base_field'] = \Drupal::service('entity_field.manager')->getBaseFieldDefinitions('redhen_contact', $bundle);
      $info['field'] = array_diff_key(\Drupal::service('entity_field.manager')->getFieldDefinitions('redhen_contact', $bundle), $info['base_field']);
    }

    $excluded_base_fields = array('id', 'revision_id', 'status');
    $base_field_options = array();
    $field_options = array();
    foreach ($info['base_field'] as $name => $field) {
      if (!in_array($name, $excluded_base_fields)) {
        $base_field_options[$name] = $field->getLabel();
      }
    }
    foreach ($info['field'] as $name => $field) {
      $field_options[$name] = $field->getLabel();
    }

    $form['base_fields'] = array(
      '#title' => t('Base fields'),
      '#type' => 'checkboxes',
      '#options' => $base_field_options,
      '#default_value' => $fields,
      '#required' => FALSE,
      '#description' => t('Selected fields will be used to query duplicates.'),
    );
    $form['fields'] = array(
      '#title' => t('Contact fields'),
      '#type' => 'checkboxes',
      '#options' => $field_options,
      '#default_value' => $fields,
      '#required' => FALSE,
      '#description' => t('Selected fields will be used to query duplicates.'),
    );
    $form['active'] = array(
      '#title' => t('Active'),
      '#type' => 'checkbox',
      '#description' => t('Limit query to active contacts.'),
      '#default_value' => $active,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );

    return $form;
  }

  public function submitForm(array &$form, \Drupal\Core\Form\FormStateInterface $form_state) {
    $query = array('base_fields' => array_filter($form_state->getValue(['base_fields'])),
      'fields' => array_filter($form_state->getValue(['fields'])),
      'active' => $form_state->getValue(['active']));
    $form_state->setRedirect('redhen_dedupe.list_page', $query);
  }

}
