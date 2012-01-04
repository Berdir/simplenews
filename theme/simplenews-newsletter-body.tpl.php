<?php

/**
 * @file
 * Default theme implementation to format the simplenews newsletter body.
 *
 * Copy this file in your theme directory to create a custom themed body.
 * Rename it to override it. Available templates:
 *   simplenews-newsletter-body--[tid].tpl.php
 *   simplenews-newsletter-body--[view mode].tpl.php
 *   simplenews-newsletter-body--[tid]--[view mode].tpl.php
 * See README.txt for more details.
 *
 * Available variables:
 * - $build: Array as expected by render().
 * - $title: Node title
 * - $language: Language object
 * - $view_mode: Active view mode.
 *
 * @see template_preprocess_simplenews_newsletter_body()
 */
?>
<h2><?php print $title; ?></h2>
<?php if ($view_mode == 'email_plain' && !empty($variables['plaintext_fieldnames'])): ?>
  <?php foreach ($variables['plaintext_fieldnames'] as $plaintext_fieldname): ?>
    <?php simplenews_html_to_text(field_view_field('node', $variables['node'], $variables['plain_field'])) ?>
  <?php endforeach; ?>
<?php else: ?>
  <?php print render($build); ?>
<?php endif; ?>
